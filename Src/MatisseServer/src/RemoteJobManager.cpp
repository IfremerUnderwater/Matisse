#include "RemoteJobManager.h"
#include <QFileInfo>
#include <QGridLayout>
#include "SshAction.h"
#include <QtWidgets\qmessagebox.h>

using namespace MatisseServer;

PasswordDialog::PasswordDialog(QWidget* parent) : 
    QDialog(parent)
{
    setupUi();
    setWindowTitle(tr("Remote host login"));
    setModal(true);
}

void PasswordDialog::setupUi()
{
    QGridLayout* layout = new QGridLayout(this);

    _lPassword = new QLabel(this);
    _lPassword->setText(tr("Password for user '%1'").arg("matisse"));

    _lePassword = new QLineEdit(this);
    _lePassword->setEchoMode(QLineEdit::Password);

    _bbButtons = new QDialogButtonBox(this);
    _bbButtons->addButton(QDialogButtonBox::Ok);
    _bbButtons->addButton(QDialogButtonBox::Cancel);

    QPushButton* okButton = _bbButtons->button(QDialogButtonBox::Ok);
    QPushButton* cancelButton = _bbButtons->button(QDialogButtonBox::Cancel);

    connect((QObject*) cancelButton, SIGNAL(clicked()), SLOT(close()));
    connect((QObject*) okButton, SIGNAL(clicked()), SLOT(onLoginAccepted()));
    
    setLayout(layout);
    layout->addWidget(_lPassword, 0, 0);
    layout->addWidget(_lePassword, 0, 1);
    layout->addWidget(_bbButtons, 1, 0, 1, 2, Qt::AlignCenter);
}

void PasswordDialog::onLoginAccepted() 
{
    QString password = _lePassword->text();
    emit userLogin(password);
    close();
}


RemoteJobManager::RemoteJobManager(QObject* parent) :
    QObject(parent),
    _host("unknown"),
    _username("noname"),
    m_pendingActionQueue()
{
}

void RemoteJobManager::init()
{
    qDebug() << "RemoteJobManager init";

    if (!_sshClient) {
        qFatal("RemoteJobManager: SSH client not initialized");
    }

    if (!_jobLauncher) {
        qFatal("RemoteJobManager: Job launcher not set");
    }

    connect(_sshClient, SIGNAL(transferFinished()), SLOT(onTransferFinished()));
    connect(_sshClient, SIGNAL(connectionFailed(SshClient::ErrorCode)), SLOT(onConnectionFailed(SshClient::ErrorCode)));

    // temp
    _host = "51.210.7.224";
    _username = "matisse";
}

void RemoteJobManager::uploadDataset(QString localDatasetDir)
{
    qDebug() << QString("Uploading dataset %1...").arg(localDatasetDir);

    SshAction* action = new UploadDirAction(_sshClient, localDatasetDir, "/home/matisse/datasets");
    m_pendingActionQueue.enqueue(action);
    //_sshClient->addAction(action);
    _checkHostAndCredentials();
}

void RemoteJobManager::uploadJobFiles(QString localJobBundleFile)
{
    qDebug() << "Uploading job files...";

    SshAction* action = new UploadFileAction(_sshClient, localJobBundleFile, "/home/matisse");
    //_sshClient->addAction(action);
    m_pendingActionQueue.enqueue(action);

    QString jobSubmitCommand = QString("qsub %1").arg("/home/matisse/aurelien.job");
    SshAction* cmdAction = new SendCommandAction(_sshClient, jobSubmitCommand);
    //_sshClient->addAction(cmdAction);
    m_pendingActionQueue.enqueue(cmdAction);

    _checkHostAndCredentials();
}

void RemoteJobManager::scheduleJob()
{
    qDebug() << "Scheduling job...";
}

void RemoteJobManager::setSshClient(SshClient *sshClient)
{
    if (!sshClient) {
        qFatal("SSH client implementation is null");
    }

    _sshClient = sshClient;
}

void RemoteJobManager::setJobLauncher(QWidget* jobLauncher)
{
    _jobLauncher = jobLauncher;
}

void RemoteJobManager::_checkHostAndCredentials()
{
    if (_hostAndCredsKnown) {
        _resumeAction();
        return;
    }

    qDebug() << "Setting remote host and credentials...";

    PasswordDialog *dialog = new PasswordDialog();
    connect(dialog, SIGNAL(userLogin(QString)), SLOT(onUserLogin(QString)));
    dialog->show();
}

void MatisseServer::RemoteJobManager::_resumeAction()
{
    /* If queue already empty, resume action after login correction */
    if (m_pendingActionQueue.isEmpty()) {
        _sshClient->resume();
        return;
    }
    
    while (!m_pendingActionQueue.isEmpty()) {
        SshAction* currentAction = m_pendingActionQueue.dequeue();
        _sshClient->addAction(currentAction);
    }
}

void RemoteJobManager::onTransferFinished() {
    qDebug() << "Receveived signal finished";
}

void RemoteJobManager::onConnectionFailed(SshClient::ErrorCode err)
{
    if (err == SshClient::ErrorCode::AuthenticationError) {
        qWarning() << "Authentication to remote host failed, retry login...";

        /* Invalidate credentials */
        _hostAndCredsKnown = false;

        _checkHostAndCredentials();
    }

    QString sshErrorString = QVariant::fromValue(err).toString();

    QMessageBox::warning(_jobLauncher, 
        tr("Connection error to remote host"), 
        tr("The connection to host '%1' failed with error code '%2'").arg(_host).arg(sshErrorString));
}

void RemoteJobManager::onUserLogin(QString password)
{
    qDebug() << "Login to remote host...";
    SshClientCredentials* creds = new SshClientCredentials(_username, password);
    _sshClient->setHost(_host);
    _sshClient->setCredentials(creds);
    _hostAndCredsKnown = true;
    _resumeAction();
}



