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
#include "SshClient.h"
#include "SshCommand.h"

using namespace MatisseCommon;

namespace MatisseServer {

class PasswordDialog : public QDialog {
  Q_OBJECT
public:
  explicit PasswordDialog(QWidget* parent = 0);
  void refreshUi();
  void setUsername(QString _username) { m_username = _username; }

protected slots:
  void sl_onLoginAccepted();

signals:
  void si_userLogin(QString password);

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
    explicit RemoteJobHelper(QObject *parent = nullptr);

    void init();

    void uploadDataset(QString localDatasetDir);
    void scheduleJob(QString _jobName, QString _localJobBundleFile);
    void downloadDataset();

    void setSshClient(SshClient *sshClient);
    void setJobLauncher(QWidget* jobLauncher);
    void setPreferences(MatissePreferences *prefs);

signals:

private slots:
  void sl_onTransferFinished();
  void sl_onShellOutputReceived(SshAction* action, QByteArray output);
  void sl_onShellErrorReceived(SshAction* action, QByteArray error);

public slots:
  void sl_onUserLogin(QString password);
  void sl_onConnectionFailed(SshClient::ErrorCode err);

private:
    QWidget* m_job_launcher = NULL;
    PasswordDialog* m_password_dialog = NULL;

    bool m_host_and_creds_known = false;
    SshClient *m_ssh_client = NULL;
    MatissePreferences* m_prefs = NULL;
    bool m_is_remote_exec_on = true;
    QQueue<SshAction*> m_pending_action_queue;
    QMap<SshAction*, MatisseTools::SshCommand*> m_commands_by_action;

    void checkHostAndCredentials();
    bool checkRemoteExecutionActive(QString customMessage);
    void resumeAction();
};

}

#endif // MATISSE_REMOTE_JOB_MANAGER_H_
