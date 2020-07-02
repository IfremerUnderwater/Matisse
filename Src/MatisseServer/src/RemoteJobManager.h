#ifndef REMOTEJOBMANAGER_H
#define REMOTEJOBMANAGER_H

#include <QObject>
#include <QtDebug>
#include <QQueue>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include "SshClient.h"


namespace MatisseServer {

class PasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit PasswordDialog(QWidget* parent = 0);

protected slots:
    void onLoginAccepted();

signals:
    void userLogin(QString password);

private:
    QLabel* _lPassword;
    QLineEdit* _lePassword;
    QDialogButtonBox* _bbButtons;

    void setupUi();
};

class RemoteJobManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteJobManager(QObject *parent = nullptr);

    void init();

    void uploadDataset(QString localDatasetDir);
    void uploadJobFiles(QString localJobBundleFile);
    void scheduleJob();
    void downloadDataset();

    void setSshClient(SshClient *sshClient);
    void setJobLauncher(QWidget* jobLauncher);

signals:


private slots:
    void onTransferFinished();

public slots:
    void onUserLogin(QString password);
    void onConnectionFailed(SshClient::ErrorCode err);

private:
    QWidget* _jobLauncher = NULL;
    bool _hostAndCredsKnown = false;
    SshClient *_sshClient = NULL;
    QString _host;
    QString _username;
    QQueue<SshAction*> m_pendingActionQueue;

    void _checkHostAndCredentials();

    void _resumeAction();

};

}

#endif // REMOTEJOBMANAGER_H
