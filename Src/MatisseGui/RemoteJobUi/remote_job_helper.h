#ifndef MATISSE_REMOTE_JOB_HELPER_H_
#define MATISSE_REMOTE_JOB_HELPER_H_

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QQueue>
#include <QtDebug>

#include "matisse_icon_factory.h"
#include "matisse_parameters_manager.h"
#include "matisse_preferences.h"
#include "matisse_remote_server_settings.h"
#include "process_data_manager.h"
#include "remote_progress_dialog.h"
#include "network_commons.h"
#include "network_client.h"
//#include "network_client_file_transfer.h"
#include "network_command.h"

using namespace network_tools;

namespace matisse {

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

    void registerNetworkFileClient(eFileTransferProtocol _protocol, NetworkClient* _client);
    void registerNetworkShellClient(eShellProtocol _protocol, NetworkClient* _client);

    void setJobLauncher(QWidget* _job_launcher);
    void setPreferences(MatissePreferences* _prefs);
    void setParametersManager(MatisseParametersManager* _param_manager);
    void setServerSettings(MatisseRemoteServerSettings* _server_settings);
    void setIconFactory(MatisseIconFactory* _icon_factory);

    QString remoteOutputPath() { return m_remote_output_path; }

signals:
    void si_jobResultsReceived(QString _job_name);
    void si_transferMessage(QString _new_message);

private slots:
    void sl_onTransferFinished(NetworkAction *_action);
    void sl_onTransferFailed(NetworkAction::eNetworkActionType _action_type, eTransferError _err);
    void sl_onDirContentsReceived(NetworkAction *_action, QList<NetworkFileInfo*> _contents);
    void sl_onCommandOutputReceived(NetworkAction* _action, QByteArray _output);
    void sl_onCommandErrorReceived(NetworkAction* _action, QByteArray _error);
    void sl_onRemotePathChanged(QString _new_path);
    void sl_onRestoreDefaultRemotePath();
    void sl_onGoToRemoteParentDir();

public slots:
    void sl_onUserLogin(QString _password);
    void sl_onUserLoginCanceled();
    void sl_onConnectionFailed(eConnectionError _err);

private:
    QWidget* m_job_launcher = NULL;
    PasswordDialog* m_password_dialog = NULL;
    RemoteProgressDialog* m_progress_dialog = NULL;
    MatisseParametersManager* m_param_manager = NULL;
    MatisseRemoteServerSettings *m_server_settings = NULL;
    MatisseIconFactory *m_icon_factory = NULL;

    QString m_remote_output_path;
    bool m_host_and_creds_known = false;
    bool m_is_last_action_command = false;

    QMap<eFileTransferProtocol, NetworkClient*> m_file_clients;
    QMap<eShellProtocol, NetworkClient*> m_shell_clients;
    NetworkClient *m_current_file_client = NULL;
    NetworkClient *m_current_shell_client = NULL;

    MatissePreferences* m_prefs = NULL;
    bool m_is_remote_exec_on = true;

    QQueue<NetworkAction*> m_pending_action_queue;
    QMap<NetworkAction*, NetworkCommand*> m_commands_by_action;
    QMap<NetworkCommand*, QString> m_jobs_by_command;
    QMap<NetworkAction*, QString> m_jobs_by_action;
    QString m_current_datasets_root_path;
    QString m_previous_datasets_root_path;
    QString m_selected_remote_dataset_path;
    QString m_selected_remote_dataset_parent_path;
    QString m_current_job_name;

    QString m_container_launcher_name; // launcher script name for server container
    QString m_container_image_path; // path to server container image

    static const QString SYMBOLIC_REMOTE_ROOT_PATH;

    void checkRemoteDirCreated();
    void connectNetworkClientSignals();
    void disconnectNetworkClientSignals();
    bool checkPreferences();
    void checkHostAndCredentials();
    bool checkRemoteExecutionActive(QString _customMessage);
    void resumeAction();
    void updateJobParameters(QString _job_name, KeyValueList _local_dataset_params, bool _is_selected_dataset=false);
    void showProgress(QString _message = QString());
    void hideProgress();
    void clearPendingActionQueue();
};

} // namespace matisse

#endif // MATISSE_REMOTE_JOB_HELPER_H_
