#ifndef MATISSE_REMOTE_JOB_MANAGER_H_
#define MATISSE_REMOTE_JOB_MANAGER_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QQueue>
#include <QtDebug>

#include "MatisseParametersManager.h"
#include "MatissePreferences.h"
#include "MatisseRemoteServerSettings.h"
#include "ProcessDataManager.h"
#include "RemoteProgressDialog.h"
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
  void resetPassword() { m_le_password->setText(""); }

protected slots:
  void sl_onLoginAccepted();
  void sl_onLoginCanceled();

signals:
  void si_userLogin(QString _password);
  void si_userLoginCanceled();

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
    void reinit();

    void uploadDataset(QString _job_name);
    void selectRemoteDataset(QString _job_name);
    void scheduleJob(QString _job_name, QString _local_job_bundle_file);
    void downloadResults(QString _job_name);

    void setSshClient(NetworkClient *_ssh_client);
    void setSftpClient(NetworkClient *_sftp_client);
    void setJobLauncher(QWidget* _job_launcher);
    void setPreferences(MatissePreferences* _prefs);
    void setDataManager(ProcessDataManager* _data_manager);
    void setParametersManager(MatisseParametersManager* _param_manager);
    void setServerSettings(MatisseRemoteServerSettings* _server_settings);

signals:
    void si_jobResultsReceived(QString _job_name);
    void si_transferMessage(QString _new_message);

private slots:
  void sl_onTransferFinished(NetworkAction *_action);
  void sl_onTransferFailed(NetworkAction* _action, NetworkClient::TransferError _err);
  void sl_onDirContentsReceived(QList<NetworkFileInfo*> _contents);
  void sl_onShellOutputReceived(NetworkAction* _action, QByteArray _output);
  void sl_onShellErrorReceived(NetworkAction* _action, QByteArray _error);

public slots:
  void sl_onUserLogin(QString password);
  void sl_onUserLoginCanceled();
  void sl_onConnectionFailed(NetworkClient::ConnectionError _err);

private:
    QWidget* m_job_launcher = NULL;
    PasswordDialog* m_password_dialog = NULL;
    RemoteProgressDialog* m_progress_dialog = NULL;
    ProcessDataManager* m_data_manager = NULL;
    MatisseParametersManager* m_param_manager = NULL;
    MatisseRemoteServerSettings *m_server_settings = NULL;

    bool m_host_and_creds_known = false;
    bool m_is_last_action_command = false;
    NetworkClient *m_ssh_client = NULL;
    NetworkClient *m_sftp_client = NULL;
    MatissePreferences* m_prefs = NULL;
    bool m_is_remote_exec_on = true;
    QQueue<NetworkAction*> m_pending_action_queue;
    QMap<NetworkAction*, MatisseTools::SshCommand*> m_commands_by_action;
    QMap<MatisseTools::SshCommand*, QString> m_jobs_by_command;
    QMap<NetworkAction*, QString> m_jobs_by_action;
    QString m_selected_remote_dataset_path;
    QString m_current_job_name;

    QString m_container_launcher_name; // launcher script name for server container
    QString m_container_image_path; // path to server container image


    void connectGatewaySignals();
    void disconnectGatewaySignals();
    bool checkPreferences();
    void checkHostAndCredentials();
    bool checkRemoteExecutionActive(QString _customMessage);
    void resumeAction();
    void updateJobParameters(QString _job_name, QString _remote_dataset_path,
                             QString _remote_nav_file);
    void showProgress(QString _message = QString());
    void hideProgress();
    void clearPendingActionQueue();
};

}

#endif // MATISSE_REMOTE_JOB_MANAGER_H_
