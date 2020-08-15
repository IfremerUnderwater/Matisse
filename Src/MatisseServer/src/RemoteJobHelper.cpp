#include "RemoteJobHelper.h"

#include <QFileInfo>
#include <QGridLayout>
#include <QMessageBox>

#include "FileUtils.h"
#include "SshAction.h"
#include "SshCommandPbsQsub.h"

using namespace MatisseCommon;
using namespace MatisseTools;

namespace MatisseServer {

PasswordDialog::PasswordDialog(QWidget* _parent) : QDialog(_parent) {
  setupUi();
  setWindowTitle(tr("Remote host login"));
  setModal(true);
}


void PasswordDialog::refreshUi() 
{
  m_la_password->setText(tr("Password for user '%1'").arg(m_username));
}


void PasswordDialog::setupUi() {
  QGridLayout* layout = new QGridLayout(this);

  m_la_password = new QLabel(this);
  m_la_password->setText(tr("Password for user '%1'").arg(m_username));

  m_le_password = new QLineEdit(this);
  m_le_password->setEchoMode(QLineEdit::Password);

  m_bb_buttons = new QDialogButtonBox(this);
  m_bb_buttons->addButton(QDialogButtonBox::Ok);
  m_bb_buttons->addButton(QDialogButtonBox::Cancel);

  QPushButton* okButton = m_bb_buttons->button(QDialogButtonBox::Ok);
  QPushButton* cancelButton = m_bb_buttons->button(QDialogButtonBox::Cancel);

  connect((QObject*)cancelButton, SIGNAL(clicked()), SLOT(reject()));
  connect((QObject*)okButton, SIGNAL(clicked()), SLOT(sl_onLoginAccepted()));

  setLayout(layout);
  layout->addWidget(m_la_password, 0, 0);
  layout->addWidget(m_le_password, 0, 1);
  layout->addWidget(m_bb_buttons, 1, 0, 1, 2, Qt::AlignCenter);
}

void PasswordDialog::sl_onLoginAccepted() {
  QString password = m_le_password->text();
  emit si_userLogin(password);
  accept();
}

RemoteJobHelper::RemoteJobHelper(QObject* _parent)
    : QObject(_parent), 
  m_pending_action_queue(), 
  m_commands_by_action(),
      m_jobs_by_command() 
{
}

void RemoteJobHelper::init() {
  qDebug() << "RemoteJobHelper init";

  if (!m_ssh_client) {
    qFatal("RemoteJobHelper: SSH client not initialized");
  }

  if (!m_job_launcher) {
    qFatal("RemoteJobHelper: Job launcher not set");
  }

  if (!m_prefs) {
    qFatal("RemoteJobHelper: Preferences not set");
  }

  /* Check preferences consistency */
  QString host = m_prefs->remoteServerAddress();
  QString username = m_prefs->remoteUsername();
  QString queue = m_prefs->remoteQueueName();
  QString defaultDataPath = m_prefs->remoteDefaultDataPath();
  QString resultPath = m_prefs->remoteResultPath();

  if (host.isEmpty()) {
    qWarning("Remote host not defined, remote execution can not be activated");
    m_is_remote_exec_on = false;
    return;
  }

  if (username.isEmpty()) {
    qWarning(
        "Remote username not defined, remote execution can not be activated");
    m_is_remote_exec_on = false;
    return;
  }

  if (queue.isEmpty()) {
    qWarning(
        "Remote execution queue not defined, remote execution can not be "
        "activated");
    m_is_remote_exec_on = false;
    return;
  }

  if (defaultDataPath.isEmpty()) {
    qWarning(
        "Remote default data path not defined, remote execution can not be "
        "activated");
    m_is_remote_exec_on = false;
    return;
  }

  if (resultPath.isEmpty()) {
    qWarning(
        "Remote result path not defined, remote execution can not be "
        "activated");
    m_is_remote_exec_on = false;
    return;
  }

  m_is_remote_exec_on = true;
  m_host_and_creds_known = false;

  connect(m_ssh_client, SIGNAL(si_transferFinished()),
          SLOT(sl_onTransferFinished()));
  connect(m_ssh_client, SIGNAL(si_connectionFailed(SshClient::ErrorCode)),
          SLOT(sl_onConnectionFailed(SshClient::ErrorCode)));
  connect(m_ssh_client, SIGNAL(si_shellOutputReceived(SshAction *, QByteArray)),
    SLOT(sl_onShellOutputReceived(SshAction*, QByteArray)));
  connect(m_ssh_client, SIGNAL(si_shellErrorReceived(SshAction*, QByteArray)),
          SLOT(sl_onShellErrorReceived(SshAction*, QByteArray)));
}

void RemoteJobHelper::uploadDataset(QString _local_dataset_dir) {
  if (!checkRemoteExecutionActive(
          tr("cannot upload dataset to remote server."))) {
    return;
  }

  qDebug() << QString("Uploading dataset %1...").arg(_local_dataset_dir);

  SshAction* action = new UploadDirAction(m_ssh_client, _local_dataset_dir,
                                          "/home/matisse/datasets");
  m_pending_action_queue.enqueue(action);
  checkHostAndCredentials();
}

void RemoteJobHelper::scheduleJob(QString _job_name, QString _local_job_bundle_file) {
  if (!checkRemoteExecutionActive(
          tr("cannot schedule job for remote execution."))) {
    return;
  }

  QFileInfo bundle_info(_local_job_bundle_file);
  if (!bundle_info.exists()) 
  {
    qCritical() << QString(
        "Job bundle file '%1' does not exist, cannot schedule job");
    return;
  }

  /* Clean and create job export dir */
  QString remote_out_path = bundle_info.absolutePath();
  QString job_export_name =
      _job_name + '_' + m_prefs->remoteUsername();
  QString job_export_path =
      remote_out_path + QDir::separator() + job_export_name;

  QDir prev_export_dir(job_export_path);
  if (prev_export_dir.exists()) 
  {
    FileUtils::removeDir(job_export_path);
  }

  QDir job_export_dir(job_export_path);  
  job_export_dir.mkpath(job_export_path);

  FileUtils::unzipFiles(_local_job_bundle_file, job_export_path);

  /* Creating PBS script file from template */
  QString pbs_template_path =
      QString("scripts") + QDir::separator() + "template_remote_job.pbs";
  QFile pbs_template(pbs_template_path);

  if (!pbs_template.exists()) 
  {
    qCritical() << QString(
        "Could not find template PBS script '%1', could not generate script "
        "for remote execution").arg(pbs_template_path);
    return;
  }

  QString target_script_file_name = job_export_name + ".pbs";

  QString target_script_path = 
      QString(job_export_path) + QDir::separator() + target_script_file_name;
  pbs_template.copy(target_script_path);

  /* Substitute script variables */
  QFile target_script_file(target_script_path);
  if (!target_script_file.open(QIODevice::ReadOnly |
                               QIODevice::Text)) {
    qCritical()
        << QString("Could not read PBS template script file %1").arg(target_script_path);
    return;
  }
  QTextStream reader(&target_script_file);
  QString template_text = reader.readAll();

  target_script_file.close();

  QString remote_job_root_path =
      QString("/home/matisse") + '/' + job_export_name;//  +"xml/jobs";

  QMap<QString, QString> variables;
  variables.insert("path.jobs.root", remote_job_root_path);
  variables.insert("job.name", _job_name);
  QString custom_text = StringUtils::substitutePlaceHolders(template_text, variables);

  /* Force Unix-syle writing (replace CRLF by LF) to ensure compatibility with PBS server */
  custom_text.remove(QRegularExpression("[\\r]"));
  if (!target_script_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCritical()
        << QString("Could not edit PBS script file %1").arg(target_script_path);
    return;
  }
  QTextStream writer(&target_script_file);
  writer << custom_text;
  writer.flush();
  target_script_file.close();

  qDebug() << "Uploading job files...";

  /* Enqueue action for job bundle upload  */
  SshAction* action =
      new UploadDirAction(m_ssh_client, job_export_path, "/home/matisse");

  m_pending_action_queue.enqueue(action);

  /* Create PBS qsub command */
  QString remote_script_path =
      QString("/home/matisse") + '/' + job_export_name + '/' + target_script_file_name;
  SshCommandPbsQsub* qsub_cmd = new SshCommandPbsQsub();
  qsub_cmd->arg(remote_script_path);
  QString qsub_cmd_text = qsub_cmd->fullCommandString();
  m_jobs_by_command.insert(qsub_cmd, _job_name);

  /* Enqueue action for qsub command */
  SshAction* qsub_cmd_action = new SendCommandAction(m_ssh_client, qsub_cmd_text);
  m_commands_by_action.insert(qsub_cmd_action, qsub_cmd);
  m_pending_action_queue.enqueue(qsub_cmd_action);

  checkHostAndCredentials();
}

void RemoteJobHelper::setSshClient(SshClient* sshClient) {
  if (!sshClient) {
    qFatal("SSH client implementation is null");
  }

  m_ssh_client = sshClient;
}

void RemoteJobHelper::setJobLauncher(QWidget* _job_launcher) {
  m_job_launcher = _job_launcher;
}

void RemoteJobHelper::setPreferences(MatissePreferences* _prefs) {
  m_prefs = _prefs;
}

void RemoteJobHelper::setDataManager(ProcessDataManager* _data_manager) 
{
  m_data_manager = _data_manager;
}

void RemoteJobHelper::checkHostAndCredentials() {
  if (m_host_and_creds_known) {
    resumeAction();
    return;
  }

  qDebug() << "Setting remote host and credentials...";

  if (!m_password_dialog) 
  {
    m_password_dialog = new PasswordDialog(m_job_launcher);
    connect(m_password_dialog, SIGNAL(si_userLogin(QString)),
            SLOT(sl_onUserLogin(QString)));
  } 

  m_password_dialog->setUsername(m_prefs->remoteUsername());
  m_password_dialog->refreshUi();
  m_password_dialog->show();
}

bool RemoteJobHelper::checkRemoteExecutionActive(QString _custom_message) {
  if (!m_is_remote_exec_on) {
    QMessageBox::warning(m_job_launcher, tr("Remote execution not activated"),
                         tr("Preferences for remote execution are not set, ")
                             .append(_custom_message));

    return false;
  }

  return true;
}

void RemoteJobHelper::resumeAction() {
  /* If queue already empty, resume action after login correction */
  if (m_pending_action_queue.isEmpty()) {
    m_ssh_client->resume();
    return;
  }

  while (!m_pending_action_queue.isEmpty()) {
    SshAction* currentAction = m_pending_action_queue.dequeue();
    m_ssh_client->addAction(currentAction);
  }
}

void RemoteJobHelper::sl_onTransferFinished() {
  qDebug() << "Receveived signal finished";
}

void RemoteJobHelper::sl_onShellOutputReceived(SshAction* _action,
                                               QByteArray _output) 
{
  if (!_action) 
  {
    qCritical() << "RemoteJobHelper: null action received, cannot interpret shell output";
    return;
  }

  SshCommand* command = m_commands_by_action.value(_action);
  if (!command) 
  {
    qCritical() << "RemoteJobHelper: shell output received, no command found for action " << _action->type();
    return;
  }

  command->appendOutputStream(_output);
  if (command->isExecuted()) 
  {
    qDebug() << QString("Command '%1' complete, terminating action.")
                    .arg(command->fullCommandString());

    /* mark action as terminated now (avoids further signalling) */
    _action->terminate();

    /* If qsub command : signal job scheduled */
    SshCommandPbsQsub* qsub_cmd = qobject_cast<SshCommandPbsQsub*>(command);
    if (qsub_cmd) {
      QString job_name = m_jobs_by_command.value(qsub_cmd);

      QDateTime timestamp = QDateTime::currentDateTime();

      /* Update job file */
      JobDefinition* job = m_data_manager->getJob(job_name);
      if (!job) {
        qCritical() << QString(
                           "Job '%1' not found, could not update remote "
                           "execution status")
                           .arg(job_name);
        return;
      }

      job->remoteJobDefinition()->setScheduled(true);
      job->remoteJobDefinition()->setNode(qsub_cmd->node());
      job->remoteJobDefinition()->setJobId(qsub_cmd->jobId());
      job->remoteJobDefinition()->setTimestamp(timestamp);

      m_data_manager->writeJobFile(job, true);

      /* Notify user */
      QMessageBox::information(
        m_job_launcher, tr("Remote job scheduling"),
        tr("Job '%1' has been scheduled for remote execution on node '%2'")
        .arg(QString(job_name))
        .arg(qsub_cmd->node()));

      m_jobs_by_command.remove(qsub_cmd);
    }

    m_commands_by_action.remove(_action);
    delete command;
  }
}

void RemoteJobHelper::sl_onShellErrorReceived(SshAction* _action,
                                              QByteArray _error) 
{
  if (!_action) {
    qCritical() << "RemoteJobHelper: null action received, cannot interpret "
                   "shell error";
    return;
  }

  SshCommand* command = m_commands_by_action.value(_action);
  if (!command) {
    qCritical() << "RemoteJobHelper: shell error received, no command found for action " << _action->type();
    return;
  }

  command->appendErrorStream(_error);
  if (command->isExecuted()) {
    qDebug() << QString("Command '%1' complete with error, terminating action.")
                    .arg(command->fullCommandString());
    m_commands_by_action.remove(_action);
    _action->terminate();
    delete command;
  }
}

void RemoteJobHelper::sl_onConnectionFailed(SshClient::ErrorCode _err) {
  if (_err == SshClient::ErrorCode::AuthenticationError) {
    qWarning() << "Authentication to remote host failed, retry login...";

    /* Invalidate credentials */
    m_host_and_creds_known = false;

    checkHostAndCredentials();
  }

  QString sshErrorString = QVariant::fromValue(_err).toString();

  QMessageBox::warning(
      m_job_launcher, tr("Connection error to remote host"),
      tr("The connection to host '%1' failed with error code '%2'")
          .arg(m_prefs->remoteServerAddress())
          .arg(sshErrorString));
}

void RemoteJobHelper::sl_onUserLogin(QString _password) {
  qDebug() << "Login to remote host...";
  SshClientCredentials* creds =
      new SshClientCredentials(m_prefs->remoteUsername(), _password);
  m_ssh_client->setHost(m_prefs->remoteServerAddress());
  m_ssh_client->setCredentials(creds);
  m_host_and_creds_known = true;
  resumeAction();
}

} // namespace MatisseServer

