#include "remote_job_helper.h"

#include <QFileInfo>
#include <QGridLayout>
#include <QMessageBox>

#include "file_utils.h"
#include "remote_file_dialog.h"
#include "network_action.h"
#include "network_action_upload_file.h"
#include "network_action_upload_dir.h"
#include "network_action_download_dir.h"
#include "network_action_dir_content.h"
#include "network_action_send_command.h"
#include "network_command_pbs_qsub.h"
#include "parameters_common.h"

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

  connect((QObject*)cancelButton, SIGNAL(clicked()), SLOT(sl_onLoginCanceled()));
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

void PasswordDialog::sl_onLoginCanceled() { 
  emit si_userLoginCanceled();
  reject();
}

RemoteJobHelper::RemoteJobHelper(QObject* _parent)
    : QObject(_parent), 
  m_pending_action_queue(), 
  m_commands_by_action(),
  m_jobs_by_command(),
  m_jobs_by_action(),
  m_selected_remote_dataset_path() 
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

  m_is_remote_exec_on = checkPreferences();
  m_host_and_creds_known = false;

  connectNetworkClientSignals();
}

void RemoteJobHelper::reinit() 
{
  m_is_remote_exec_on = checkPreferences(); /* check preferences completeness */
  if (!m_is_remote_exec_on) {
    return;
  }

  if (!m_host_and_creds_known) { /* no connection to remote hosts have occurred yet */
    return;
  }

  /* Check if connection preferences have changed */
  QString current_ssh_host = m_ssh_client->host();
  QString current_username = m_ssh_client->username(); /* username identical for SSH and SFTP servers */
  QString current_sftp_host = m_sftp_client->host();

  bool host_and_creds_changed = false;

  if (m_prefs->remoteCommandServer() != current_ssh_host
    || m_prefs->remoteFileServer() != current_sftp_host
    || m_prefs->remoteUsername() != current_username) {
    host_and_creds_changed = true;
    m_host_and_creds_known = false;
  }

  if (host_and_creds_changed) {
    qDebug() << "RemoteJobHelper: remote connection preferences have changed, "
                "any ongoing action will be interrupted...";

    hideProgress(); /* Any ongoing action will be interrupted */

    /* reset connections */
    m_sftp_client->resetConnection();
    m_ssh_client->resetConnection();

    clearPendingActionQueue();
  }

}


void RemoteJobHelper::uploadDataset(QString _job_name) {
  if (!checkRemoteExecutionActive(
          tr("cannot upload dataset to remote server."))) {
    return;
  }

  m_current_job_name = _job_name;
  m_selected_remote_dataset_path = "";

  /* Prompt user to select local dataset */
  QSettings settings("IFREMER", "Matisse3D");
  QVariant ul_path_setting = settings.value("remote/localUploadPath");

  QFileDialog dialog(m_job_launcher, tr("Select dataset to upload"));
  dialog.setFileMode(QFileDialog::Directory);
  if (!ul_path_setting.isNull()) {
    QString ul_path = ul_path_setting.toString();
    dialog.setDirectory(ul_path);
  }

  if (!dialog.exec()) {
    qCritical() << "File dialog could no execute";
    return;
  }

  QStringList filenames = dialog.selectedFiles();
  if (filenames.isEmpty()) {
    qCritical() << "No dataset folder selected, upload could not be performed";
    return;
  }

  QString local_dataset_dir_name = filenames.at(0);

  if (local_dataset_dir_name.isEmpty()) {
    qCritical() << "Selected dataset folder name is empty, upload could not be "
                   "performed";
    return;
  }

  QDir local_dataset_dir(local_dataset_dir_name);
  if (!local_dataset_dir.exists()) {
    qCritical() << QString(
                       "Dataset folder %1 does not exist, upload could not be "
                       "performed")
                       .arg(local_dataset_dir_name);
    return;
  }

  QString local_dataset_dir_path = local_dataset_dir.canonicalPath();

  /* Persist selected folder */
  QVariant new_ul_path_setting(local_dataset_dir_path);
  settings.setValue("remote/localUploadPath", new_ul_path_setting);

  qDebug() << QString("Uploading dataset %1...").arg(local_dataset_dir_path);

  /* Remove existing result files if present */
  QStringList subdirs = local_dataset_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  if (!subdirs.isEmpty()) {
    QMessageBox::StandardButton answer = QMessageBox::question(
        m_job_launcher, tr("Remove unnecessary files from dataset directory"),
        tr("Previous result files shall not be uploaded.\nRemove all "
           "subdirectories from local dataset directory '%1' ?")
            .arg(local_dataset_dir_path));

    if (answer == QMessageBox::No) {
      qWarning() << "Dataset upload cancelled by user : refused to remove result files from dataset dir";
      return;
    }

    for (QString subdir_name : subdirs) {
      qDebug() << "Removing dataset subdir " << subdir_name;
      QString subdir_path = local_dataset_dir_path + QDir::separator() + subdir_name;
      QDir subdir(subdir_path);
      bool status = subdir.removeRecursively();
      if (!status) {
        qWarning() << QString("Dataset subdir '%1' could not be deleted, dataset will be uploaded anyway").arg(subdir_name);
      }
    }
    
  }

  /* Check for navigation files */
  QStringList name_filters;
  name_filters << "*.dim2" << "*.txt";
  QStringList nav_files = local_dataset_dir.entryList(name_filters, QDir::Files);

  if (nav_files.isEmpty()) {
    QMessageBox::warning(
      m_job_launcher, 
      tr("Navigation file not found"),
      tr("No navigation file (*.dim2 or *.txt) was found in the selected dataset dir '%1'.\nThe navigation file must be included in the uploaded dataset dir.")
            .arg(local_dataset_dir_path));
    return;
  }

  QString selected_nav_file = QFileDialog::getOpenFileName(
    m_job_launcher, 
    tr("Select navigation file"), local_dataset_dir_path,
    tr("Navigation Files (%1)").arg(name_filters.join(" ")));

  if (selected_nav_file.isEmpty()) {
    qWarning("Navigation file selection canceled by user, dataset cannot be uploaded");
    return;
  }

  QFileInfo nav_file_info(selected_nav_file);
  QString nav_file_name = nav_file_info.fileName();

  /* Update job parameter file */
  QString dataset_root_dir_bound = m_server_settings->datasetsPathBound();
  QString dir_name = local_dataset_dir.dirName();
  QString remote_dataset_dir = dataset_root_dir_bound + '/' + dir_name;
  
  updateJobParameters(_job_name, remote_dataset_dir, nav_file_name);

  /* Create and enqueue upload action */
  NetworkAction* action =
      new NetworkActionUploadDir(local_dataset_dir_path,
                                 m_server_settings->datasetsPath());
  action->setMetainfo("dataset");
  m_pending_action_queue.enqueue(action);
  m_jobs_by_action.insert(action, _job_name);
  checkHostAndCredentials();
}

void RemoteJobHelper::selectRemoteDataset(QString _job_name) {
  if (!checkRemoteExecutionActive(
          tr("Cannot select remote dataset on server."))) {
    return;
  }

  qDebug() << QString("Selecting dataset for job '%1'...").arg(_job_name);

  m_current_job_name = _job_name;
  m_selected_remote_dataset_path = "";

  NetworkAction* action =
      new NetworkActionDirContent(m_server_settings->datasetsPath(),
                               FileTypeFilter::Dirs);

  m_pending_action_queue.enqueue(action);
  checkHostAndCredentials();
}

void RemoteJobHelper::scheduleJob(QString _job_name, QString _local_job_bundle_file) {
  if (!checkRemoteExecutionActive(
          tr("cannot schedule job for remote execution."))) {
    return;
  }

  m_current_job_name = _job_name;
  m_selected_remote_dataset_path = "";

  QFileInfo bundle_info(_local_job_bundle_file);
  if (!bundle_info.exists()) 
  {
    qCritical() << QString(
        "Job bundle file '%1' does not exist, cannot schedule job");
    return;
  }

  /* Clean and create job export dir */
  QString remote_out_path = bundle_info.absolutePath();
  QString job_export_name = _job_name + '_' + m_prefs->remoteUsername();
  QString job_export_path = remote_out_path + QDir::separator() + job_export_name;

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

  /* Substitute PBS script variables */
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

  QString remote_jobs_root = m_server_settings->jobsPath();
  QString remote_job_bundle_path = remote_jobs_root + '/' + job_export_name;
  QString remote_job_bundle_path_bound = m_server_settings->jobsPathBound() + '/' + job_export_name;

  QMap<QString, QString> variables;
  variables.insert("server.container.image", m_server_settings->containerImage().path());
  variables.insert("launch.script.path", m_server_settings->launcherParentDir().path());
  variables.insert("launch.script.binding", m_server_settings->launcherParentDir().alias());
  variables.insert("launch.script.name", m_server_settings->launcherFile().path());
  variables.insert("bin.root.path", m_server_settings->binRoot().path());
  variables.insert("bin.root.binding", m_server_settings->binRoot().alias());
  variables.insert("runtime.files.root.path", m_server_settings->applicationFilesRoot().path());
  variables.insert("runtime.datasets.subdir", m_server_settings->datasetsSubdir().path());
  variables.insert("runtime.datasets.binding", m_server_settings->datasetsSubdir().alias());
  variables.insert("runtime.jobs.subdir", m_server_settings->jobsSubdir().path());
  variables.insert("runtime.jobs.binding", m_server_settings->jobsSubdir().alias());
  variables.insert("runtime.results.subdir", m_server_settings->resultsSubdir().path());
  variables.insert("runtime.results.binding", m_server_settings->resultsSubdir().alias());
  variables.insert("jobs.root.path", remote_job_bundle_path);
  variables.insert("jobs.root.path.bound", remote_job_bundle_path_bound);
  variables.insert("job.name", _job_name);
  variables.insert("job.export.name", job_export_name);
  variables.insert("scheduler.queue.name", m_prefs->remoteQueueName());
  variables.insert("job.resources.ncpus", QVariant(m_prefs->remoteNbOfCpus()).toString());
  variables.insert("job.owner.email", m_prefs->remoteUserEmail());
  
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
  NetworkAction* job_upload_action =
      new NetworkActionUploadDir(job_export_path, remote_jobs_root);
  job_upload_action->setMetainfo("job");
  m_pending_action_queue.enqueue(job_upload_action);
  m_jobs_by_action.insert(job_upload_action, _job_name);

  /* Create PBS qsub command */
  QString remote_script_path =
      remote_job_bundle_path + '/' + target_script_file_name;
  NetworkCommandPbsQsub* qsub_cmd = new NetworkCommandPbsQsub();
  qsub_cmd->arg(remote_script_path);
  QString qsub_cmd_text = qsub_cmd->fullCommandString();
  m_jobs_by_command.insert(qsub_cmd, _job_name);

  /* Enqueue action for qsub command */
  NetworkAction* qsub_cmd_action = new NetworkActionSendCommand(qsub_cmd_text);
  m_commands_by_action.insert(qsub_cmd_action, qsub_cmd);
  m_pending_action_queue.enqueue(qsub_cmd_action);

  checkHostAndCredentials();
}

void RemoteJobHelper::downloadResults(QString _job_name) 
{
  JobDefinition* job = ProcessDataManager::instance()->getJob(_job_name);
  RemoteJobDefinition* remote = job->remoteJobDefinition();

  if (!remote->isScheduled()) {
    QMessageBox::warning(m_job_launcher, tr("Job not scheduled"),
                         tr("Job '%1' was never scheduled for execution on remote server."
                           "\nThere are no results to be downloaded.").arg(_job_name));
    return;
  }

  /* Retrieve bound result path from parameters */
  KeyValueList kvl;
  kvl.insert(DATASET_PARAM_OUTPUT_DIR, "");
  m_param_manager->pullDatasetParameters(kvl);

  QString remote_dir_path = kvl.getValue(DATASET_PARAM_OUTPUT_DIR);

  /* Substitute container bound result path by real path */
  remote_dir_path.replace(m_server_settings->resultsPathBound(), m_server_settings->resultsPath());

  /* Open dialog for user to select target folder */
  QSettings settings("IFREMER", "Matisse3D");
  QVariant dl_path_setting = settings.value("remote/localDownloadPath");

  QFileDialog dialog(m_job_launcher, tr("Select results destination folder"));
  dialog.setFileMode(QFileDialog::Directory);
  if (!dl_path_setting.isNull()) {
    QString dl_path = dl_path_setting.toString();
    dialog.setDirectory(dl_path);
  }

  if (!dialog.exec()) {
    qCritical() << "RemoteJobHelper: file dialog could no execute";
    return;
  }

  QStringList filenames = dialog.selectedFiles();
  if (filenames.isEmpty()) {
    qCritical() << "No results destination folder selected, upload could not be performed";
    return;
  }

  QString results_dest_dir_name = filenames.at(0);

  if (results_dest_dir_name.isEmpty()) {
    qCritical() << "Selected results destination folder name is empty, download could not be "
                   "performed";
    return;
  }

  QDir results_dest_dir(results_dest_dir_name);
  if (!results_dest_dir.exists()) {
    qCritical() << QString(
                       "Results destination folder %1 does not exist, download could not be "
                       "performed")
                       .arg(results_dest_dir_name);
    return;
  }

  /* Persist selected folder */
  QVariant new_dl_path_setting(results_dest_dir_name);
  settings.setValue("remote/localDownloadPath", new_dl_path_setting);

  /* Prepare download */
  QString local_base_dir_path = results_dest_dir.canonicalPath() + QDir::separator() + _job_name;
  QDir local_base_dir(local_base_dir_path);
  if (!local_base_dir.exists()) {
    local_base_dir.mkpath(".");
  } else {
    /* Check if previous results are present */
    QStringList old_files = local_base_dir.entryList(QDir::Files);

    if (!old_files.isEmpty()) {
      QMessageBox::StandardButton answer = QMessageBox::question(
          m_job_launcher, tr("Clear previous results before downloading"),
          tr("Previous result files (remote execution) were found for the job '%1'.\n"
            "They will be deleted before downloading new results.\nContinue ?")
              .arg(_job_name));

      if (answer == QMessageBox::No) {
        qWarning() << "Results download cancelled by user : refused to remove "
                      "previous result files";
        return;
      }

      /* Remove previous results */
      for (QString old_file : old_files) {
        bool removed = local_base_dir.remove(old_file);
        if (!removed) {
          qCritical() << QString("Failed to remove previous result file '%1'. This may prevent from downloading new result file").arg(old_file);
        }
      }
    }
  }

  NetworkAction *action = new NetworkActionDownloadDir(remote_dir_path, local_base_dir_path);
  m_pending_action_queue.enqueue(action);
  m_jobs_by_action.insert(action, _job_name);

  checkHostAndCredentials();
}

void RemoteJobHelper::setSshClient(NetworkClient* _ssh_client) {
  if (!_ssh_client) {
    qFatal("SSH client implementation is null");
  }

  m_ssh_client = _ssh_client;
}

void RemoteJobHelper::setSftpClient(NetworkClient* _sftp_client) {
  if (!_sftp_client) {
    qFatal("SFTP client implementation is null");
  }

  m_sftp_client = _sftp_client;
}

void RemoteJobHelper::setJobLauncher(QWidget* _job_launcher) {
  m_job_launcher = _job_launcher;
}

void RemoteJobHelper::setPreferences(MatissePreferences* _prefs) {
  m_prefs = _prefs;
}

void RemoteJobHelper::setParametersManager(
    MatisseParametersManager* _param_manager) 
{
  m_param_manager = _param_manager;
}

void RemoteJobHelper::setServerSettings(MatisseRemoteServerSettings* _server_settings) {
  m_server_settings = _server_settings;
}

void RemoteJobHelper::connectNetworkClientSignals() {
  connect(m_sftp_client,
          SIGNAL(si_connectionFailed(ConnectionError)),
          SLOT(sl_onConnectionFailed(ConnectionError)));
  connect(m_sftp_client, SIGNAL(si_transferFinished(NetworkAction*)),
          SLOT(sl_onTransferFinished(NetworkAction*)));
  connect(m_sftp_client,
          SIGNAL(si_transferFailed(NetworkAction*, TransferError)),
          SLOT(sl_onTransferFailed(NetworkAction*, TransferError)));
  connect(m_sftp_client, SIGNAL(si_dirContents(QList<NetworkFileInfo*>)),
          SLOT(sl_onDirContentsReceived(QList<NetworkFileInfo*>)));

  connect(m_ssh_client, SIGNAL(si_connectionFailed(ConnectionError)),
          SLOT(sl_onConnectionFailed(ConnectionError)));
  connect(m_ssh_client, SIGNAL(si_shellOutputReceived(NetworkAction*, QByteArray)),
          SLOT(sl_onShellOutputReceived(NetworkAction*, QByteArray)));
  connect(m_ssh_client, SIGNAL(si_shellErrorReceived(NetworkAction*, QByteArray)),
          SLOT(sl_onShellErrorReceived(NetworkAction*, QByteArray)));
}

void RemoteJobHelper::disconnectNetworkClientSignals() {
  disconnect(this, SLOT(sl_onConnectionFailed(ConnectionError)));
  disconnect(this, SLOT(sl_onTransferFinished(NetworkAction*)));
  disconnect(this, SLOT(sl_onTransferFailed(NetworkAction*, TransferError)));
  disconnect(this, SLOT(sl_onDirContentsReceived(QList<NetworkFileInfo*>)));
  disconnect(this, SLOT(sl_onShellOutputReceived(NetworkAction*, QByteArray)));
  disconnect(this, SLOT(sl_onShellErrorReceived(NetworkAction*, QByteArray)));
  
  disconnect(m_sftp_client, SIGNAL(si_connectionFailed(ConnectionError)));
  disconnect(m_sftp_client, SIGNAL(si_transferFinished(NetworkAction*)));
  disconnect(m_sftp_client, SIGNAL(si_transferFailed(NetworkAction*, TransferError)));
  disconnect(m_sftp_client, SIGNAL(si_dirContents(QList<NetworkFileInfo*>)));

  disconnect(m_ssh_client, SIGNAL(si_connectionFailed(ConnectionError)));
  disconnect(m_ssh_client, SIGNAL(si_shellOutputReceived(NetworkAction*, QByteArray)));
  disconnect(m_ssh_client, SIGNAL(si_shellErrorReceived(NetworkAction*, QByteArray)));
}

bool RemoteJobHelper::checkPreferences() {

  /* Check preferences consistency */
  QString ssh_host = m_prefs->remoteCommandServer();
  QString sftp_host = m_prefs->remoteFileServer();
  QString username = m_prefs->remoteUsername();
  QString queue = m_prefs->remoteQueueName();
  QString email = m_prefs->remoteUserEmail();
  int ncpus = m_prefs->remoteNbOfCpus();

  if (ssh_host.isEmpty()) {
    qWarning(
        "Remote command host not defined, remote execution can not be "
        "activated");
    return false;
  }

  if (sftp_host.isEmpty()) {
    qWarning(
        "Remote file host not defined, remote execution can not be activated");
    return false;
  }

  if (username.isEmpty()) {
    qWarning(
        "Remote username not defined, remote execution can not be activated");
    return false;
  }

  if (queue.isEmpty()) {
    qWarning(
        "Remote execution queue not defined, remote execution can not be "
        "activated");
    return false;
  }

  if (email.isEmpty()) {
    qWarning(
        "Notification email for remote execution not defined, remote execution "
        "can not be "
        "activated");
    return false;
  }

  if (ncpus < 1) {
    qWarning(
        "Nb of CPUs for remote execution not properly set, remote execution "
        "can not be "
        "activated");
    return false;
  }

  return true;
}

void RemoteJobHelper::checkHostAndCredentials() {
  if (m_host_and_creds_known) {
     resumeAction();
    return;
  }

  hideProgress();

  qDebug() << "Setting remote host and credentials...";

  if (!m_password_dialog) 
  {
    m_password_dialog = new PasswordDialog(m_job_launcher);
    connect(m_password_dialog, SIGNAL(si_userLogin(QString)), SLOT(sl_onUserLogin(QString)));
    connect(m_password_dialog, SIGNAL(si_userLoginCanceled()), SLOT(sl_onUserLoginCanceled()));
  } 

  m_password_dialog->setUsername(m_prefs->remoteUsername());
  m_password_dialog->resetPassword();
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
    if (m_is_last_action_command) {
      m_ssh_client->resume();
    } else {
      m_sftp_client->resume();
    }

    showProgress();
    return;
  }

  if (!m_pending_action_queue.isEmpty()) {
    NetworkAction* current_action = m_pending_action_queue.dequeue();
    /* Dispatch action to appropriate handler according to its type */
    if (current_action->type() == NetworkAction::NetworkActionType::SendCommand) {
      m_ssh_client->addAction(current_action);
      m_is_last_action_command = true;
    } else {
      m_sftp_client->addAction(current_action);
      m_is_last_action_command = false;
    }

    showProgress(current_action->progressMessage());
  }
}

void RemoteJobHelper::updateJobParameters(QString _job_name,
                                          QString _remote_dataset_path,
                                          QString _nav_file) 
{
  
  QString remote_nav_file_path = (_nav_file.isEmpty()) ? "" :
    _remote_dataset_path + '/' + _nav_file;
  QString job_export_name = _job_name + '_' + m_prefs->remoteUsername();
  QString job_result_path = m_server_settings->resultsPathBound() + '/' + job_export_name;

  KeyValueList kvl;
  kvl.set(DATASET_PARAM_DATASET_DIR, _remote_dataset_path);
  kvl.set(DATASET_PARAM_NAVIGATION_FILE, remote_nav_file_path);
  kvl.set(DATASET_PARAM_OUTPUT_DIR, job_result_path);
  kvl.set(DATASET_PARAM_OUTPUT_FILENAME,
          m_prefs->defaultMosaicFilenamePrefix());
  qDebug() << "Dataset params :\n" << kvl.getKeys();
  m_param_manager->pushDatasetParameters(kvl);
  m_param_manager->saveParametersValues(_job_name, false);
}

void RemoteJobHelper::showProgress(QString _message) 
{ 
  if (!m_progress_dialog) {
    m_progress_dialog = new RemoteProgressDialog(m_job_launcher);
    m_progress_dialog->setModal(false);
    connect(m_ssh_client, SIGNAL(si_progressUpdate(int)), m_progress_dialog, SLOT(sl_onProgressUpdate(int)));
    connect(m_sftp_client, SIGNAL(si_progressUpdate(int)), m_progress_dialog, SLOT(sl_onProgressUpdate(int)));
    connect(this, SIGNAL(si_transferMessage(QString)), m_progress_dialog,
            SLOT(sl_onMessageUpdate(QString)), Qt::QueuedConnection);
  } else {
    m_progress_dialog->reinit();
  }

  if (!_message.isEmpty()) { // do not update transfer info if no message is provided
    emit si_transferMessage(_message);
  }

  m_progress_dialog->show();
}

void RemoteJobHelper::hideProgress() 
{ 
  if (m_progress_dialog) {
    m_progress_dialog->hide(); 
  }
}

void RemoteJobHelper::clearPendingActionQueue() {

  qDebug() << "RemoteJobHelper: clearing SSH actions, SSH commands and links to jobs...";

  while (!m_pending_action_queue.isEmpty()) {
    NetworkAction* action = m_pending_action_queue.dequeue();

    if (m_commands_by_action.contains(action)) {
      NetworkCommand* command = m_commands_by_action.value(action);
      if (m_jobs_by_command.contains(command)) {
        m_jobs_by_command.remove(command);      
      }

      m_commands_by_action.remove(action);
      delete command;
    }

    if (m_jobs_by_action.contains(action)) {
      m_jobs_by_action.remove(action);
    }

    delete action;
  }
}

void RemoteJobHelper::sl_onTransferFinished(NetworkAction *_action) {
  qDebug() << "Receveived signal transfer finished";

  hideProgress();
   
  QString job_name = m_jobs_by_action.value(_action);
  
  if (job_name.isEmpty()) {
    qCritical() << "Could not find job for completed action "
                << _action->type();
    return;
  }
  
  if (_action->type() == NetworkAction::NetworkActionType::DownloadDir) {

    NetworkActionDownloadDir* dl_action = static_cast<NetworkActionDownloadDir*>(_action);

    ProcessDataManager* data_manager = ProcessDataManager::instance();

    /* Updating job file with result files */
    JobDefinition* job = data_manager->getJob(job_name);
    ExecutionDefinition* exe = job->executionDefinition();
    exe->setExecuted(true);
    exe->setExecutionDate(
        QDateTime::currentDateTime());  // default timestamp, not knowing the
                                        // server timestamp

    QString local_results_dir_name = dl_action->localBaseDir();
    QDir local_results_dir(local_results_dir_name);
    QStringList name_filters;
    name_filters << "*.tiff";
    QFileInfoList result_files =
        local_results_dir.entryInfoList(name_filters, QDir::Files, QDir::Name);
    QStringList result_file_paths;
    for (QFileInfo result_file : result_files) {
      result_file_paths << result_file.absoluteFilePath();
    }

    exe->setResultFileNames(result_file_paths);

    data_manager->writeJobFile(job, true);

    QMessageBox::information(
        m_job_launcher, tr("Transfer complete"),
        tr("Results file for job '%1' were downloaded successfully to '%2'")
            .arg(job_name)
            .arg(dl_action->localBaseDir()));

    /* Notify parent UI */
    emit si_jobResultsReceived(job_name);

  } else if (_action->type() == NetworkAction::NetworkActionType::UploadDir) {

    NetworkActionUploadDir* ul_action = static_cast<NetworkActionUploadDir*>(_action);

    if (ul_action->metainfo() == "dataset") {
      QMessageBox::information(
          m_job_launcher, tr("Transfer complete"),
          tr("Dataset files for job '%1' were uploaded successfully to '%2'")
              .arg(job_name)
              .arg(ul_action->remoteBaseDir()));

    } else if (ul_action->metainfo() == "job") {
      /* If action was uploading job files, go on with job scheduling */
      qDebug() << QString("Job files uploaded for job '%1', now scheduling...")
                      .arg(job_name);
      resumeAction();
    }

  } else {

    qDebug() << "Completed operation for unexpected action " << _action->type();
  }
}

void RemoteJobHelper::sl_onTransferFailed(NetworkAction* _action,
                                          TransferError _err)
{
    Q_UNUSED(_action)

  QString failed_host = m_prefs->remoteFileServer();

  hideProgress();  // in case of timeout

  QString error_str = QVariant::fromValue(_err).toString();

  QMessageBox::warning(
      m_job_launcher, tr("Transfer failed"),
      tr("Transfer to/from host '%1' failed with error code '%2'")
          .arg(failed_host)
          .arg(error_str));
}

void RemoteJobHelper::sl_onDirContentsReceived(QList<NetworkFileInfo*> _contents) {
  qDebug() << "Received remote dir contents";

  hideProgress();

  if (m_selected_remote_dataset_path.isEmpty()) {
    /* First round : selecting dataset dir */
    QString data_root_folder = m_server_settings->datasetsPath();

    RemoteFileTreeModelFactory factory;
    TreeModel* model = factory.createModel(data_root_folder, _contents);
    RemoteFileDialog rfd(model, m_job_launcher);
    if (!rfd.exec()) {
      qWarning() << "Dataset was not selected";
      return;
    }

    QString selected_dataset = rfd.selectedFile();
    qDebug() << "Selected dataset: " << selected_dataset;

    m_selected_remote_dataset_path = data_root_folder + '/' + selected_dataset;

    QStringList name_filters;
    name_filters << "*.dim2"
                 << "*.txt";

    NetworkAction* action = new NetworkActionDirContent(
        m_selected_remote_dataset_path,
        FileTypeFilter::Files, name_filters);

    m_pending_action_queue.enqueue(action);
    resumeAction();

  } else { // dataset dir already selected

    /* Second round : selecting navigation file */
    QString selected_navfile("");

    if (!_contents.isEmpty()) {
      RemoteFileTreeModelFactory factory;
      TreeModel* model = factory.createModel(m_selected_remote_dataset_path, _contents);
      RemoteFileDialog rfd(model, m_job_launcher);
      
      if (rfd.exec()) {
        selected_navfile = rfd.selectedFile();
      } else { // selection canceled by user
        QMessageBox::warning(
            m_job_launcher, tr("Remote navigation file not selected"),
            tr("No navigation file was selected for remote dataset dir '%1'.\nPlease specify parameter manually.")
                .arg(m_selected_remote_dataset_path));
      }

    } else { // no nav file found in remote dataset dir
      QMessageBox::warning(
          m_job_launcher, tr("Remote navigation file not found"),
          tr("No navigation file (*.dim2 or *.txt) was found in the selected "
             "dataset dir '%1'.\nPlease specify parameter manually.")
              .arg(m_selected_remote_dataset_path));
    }

    /* Substitute OS path for datasets root by container bound path */
    QString current_dataset_path_bound = m_selected_remote_dataset_path;
    current_dataset_path_bound.replace(m_server_settings->datasetsPath(), m_server_settings->datasetsPathBound());

    updateJobParameters(m_current_job_name, current_dataset_path_bound, selected_navfile);
  }
}

void RemoteJobHelper::sl_onShellOutputReceived(NetworkAction* _action,
                                               QByteArray _output) 
{
  if (!_action) 
  {
    qCritical() << "RemoteJobHelper: null action received, cannot interpret shell output";
    return;
  }

  NetworkCommand* command = m_commands_by_action.value(_action);
  if (!command) 
  {
    qCritical() << "RemoteJobHelper: shell output received, no command found for action " << _action->type();
    return;
  }

  command->appendOutputStream(_output);
  if (command->isExecuted()) 
  {
    hideProgress();
    
    bool success = command->isSuccessfull();
    
    qDebug() << QString("Command '%1' complete, terminating action.")
                    .arg(command->fullCommandString());

    /* mark action as terminated now (avoids further signalling) */
    _action->terminate();

    /* If qsub command : signal job scheduled */
    NetworkCommandPbsQsub* qsub_cmd = qobject_cast<NetworkCommandPbsQsub*>(command);
    if (qsub_cmd) {

      QString job_name = m_jobs_by_command.value(qsub_cmd);
      
      if (success) {

        QDateTime timestamp = QDateTime::currentDateTime();

        /* Update job file */
        ProcessDataManager* data_manager = ProcessDataManager::instance();

        JobDefinition* job = data_manager->getJob(job_name);
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

        data_manager->writeJobFile(job, true);

        /* Notify user */
        QMessageBox::information(
            m_job_launcher, tr("Remote job scheduling"),
            tr("Job '%1' has been scheduled for remote execution on node '%2'")
                .arg(QString(job_name))
                .arg(qsub_cmd->node()));
      } else {
        QMessageBox::warning(
            m_job_launcher, tr("Remote job scheduling error"),
            tr("Job '%1' could not be scheduled.\nPlease contact your administrator")
                .arg(QString(job_name)));
      }

      m_jobs_by_command.remove(qsub_cmd);
    }

    m_commands_by_action.remove(_action);
    delete command;
  }
}

void RemoteJobHelper::sl_onShellErrorReceived(NetworkAction* _action,
                                              QByteArray _error) 
{
  if (!_action) {
    qCritical() << "RemoteJobHelper: null action received, cannot interpret "
                   "shell error";
    return;
  }

  NetworkCommand* command = m_commands_by_action.value(_action);
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

void RemoteJobHelper::sl_onConnectionFailed(ConnectionError _err) {
  
  QObject* emitter = sender();

  QString failed_host = (emitter == m_ssh_client) ? 
    m_prefs->remoteCommandServer() : m_prefs->remoteFileServer();

  if (_err == ConnectionError::AuthenticationError) {
    qWarning() << QString("Authentication to remote host '%1' failed, retry login...").arg(failed_host);

    /* Invalidate credentials */
    m_host_and_creds_known = false;

    checkHostAndCredentials();
  }

  hideProgress(); // in case of timeout

  QString error_str = QVariant::fromValue(_err).toString();

  QMessageBox::warning(
      m_job_launcher, tr("Connection error to remote host"),
      tr("The connection to host '%1' failed with error code '%2'")
          .arg(failed_host)
          .arg(error_str));
}

void RemoteJobHelper::sl_onUserLogin(QString _password) {
  qDebug() << "Login to remote host...";
  NetworkClientCredentials* creds =
      new NetworkClientCredentials(m_prefs->remoteUsername(), _password);
  
  /* Assume credentials are the same on both SFTP and SSH server */
  m_sftp_client->setHost(m_prefs->remoteFileServer());
  m_sftp_client->setCredentials(creds);
  m_ssh_client->setHost(m_prefs->remoteCommandServer());
  m_ssh_client->setCredentials(creds);
  m_host_and_creds_known = true;
  resumeAction();
}

void RemoteJobHelper::sl_onUserLoginCanceled() {
  qDebug() << "Login canceled by user, remote operation aborted...";
  clearPendingActionQueue();
  m_sftp_client->clearActions();
  m_ssh_client->clearActions();
  m_is_last_action_command = false;
}

} // namespace MatisseServer

