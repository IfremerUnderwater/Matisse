#ifndef MATISSE_REMOTE_JOB_MANAGER_H_
#define MATISSE_REMOTE_JOB_MANAGER_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QQueue>
#include <QtDebug>

#include "MatissePreferences.h"
#include "ProcessDataManager.h"
#include "SshClient.h"
#include "SshCommand.h"

using namespace MatisseCommon;
using namespace MatisseTools;

namespace MatisseServer {

class PasswordDialog : public QDialog {
  Q_OBJECT
public:
  explicit PasswordDialog(QWidget* _parent = 0);
  void refreshUi();
  void setUsername(QString _username) { m_username = _username; }

protected slots:
  void sl_onLoginAccepted();

signals:
  void si_userLogin(QString _password);

private:
  QLabel* m_la_password;
  QLineEdit* m_le_password;
  QDialogButtonBox* m_bb_buttons;

  QString m_username = "noname";

  void setupUi();
};

class RemoteJobHelper : public QObject
{
    Q_OBJECT
public:
    explicit RemoteJobHelper(QObject *_parent = nullptr);

    void init();

    void uploadDataset(QString _local_dataset_dir);
    void scheduleJob(QString _job_name, QString _local_job_bundle_file);
    void downloadDataset();

    void setSshClient(SshClient *_ssh_client);
    void setJobLauncher(QWidget* _job_launcher);
    void setPreferences(MatissePreferences* _prefs);
    void setDataManager(ProcessDataManager* _data_manager);

signals:

private slots:
  void sl_onTransferFinished();
  void sl_onShellOutputReceived(SshAction* _action, QByteArray _output);
  void sl_onShellErrorReceived(SshAction* _action, QByteArray _error);

public slots:
  void sl_onUserLogin(QString password);
  void sl_onConnectionFailed(SshClient::ErrorCode _err);

private:
    QWidget* m_job_launcher = NULL;
    PasswordDialog* m_password_dialog = NULL;
    ProcessDataManager* m_data_manager = NULL;

    bool m_host_and_creds_known = false;
    SshClient *m_ssh_client = NULL;
    MatissePreferences* m_prefs = NULL;
    bool m_is_remote_exec_on = true;
    QQueue<SshAction*> m_pending_action_queue;
    QMap<SshAction*, MatisseTools::SshCommand*> m_commands_by_action;
    QMap<MatisseTools::SshCommand*, QString> m_jobs_by_command;

    void checkHostAndCredentials();
    bool checkRemoteExecutionActive(QString _customMessage);
    void resumeAction();
};

}

#endif // MATISSE_REMOTE_JOB_MANAGER_H_
