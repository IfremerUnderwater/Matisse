#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QObject>
#include <QtDebug>
#include <QQueue>
#include "SshAction.h"

namespace MatisseCommon {

class SshClientCredentials
{
public:
    explicit SshClientCredentials(QString _username, QString _password);

    QString username() { return _username; };
    QString password() { return _password; };

private:
    QString _username;
    QString _password;
};

class SshActionManager : public QObject {
    friend class UploadFileAction;
    friend class UploadDirAction;
    friend class SendCommandAction;

    Q_OBJECT

public:
    explicit SshActionManager();


protected:
    virtual void createSftpChannel() = 0;
    virtual void createRemoteShell(QString& command) = 0;
    virtual void createRemoteProcess(QString &command) = 0;
    virtual void executeCommand() = 0;
    virtual void upload(QString localPath, QString remotePath, bool isDirUpload = false) = 0; // récupérer flux
    virtual void uploadDir(QString localDir, QString remoteBaseDir) = 0;
};

class SshAction;


class SshClient : public SshActionManager
{
    Q_OBJECT

public:
    /*!
     * \brief SSH specific errors
     */
    enum class ErrorCode {
        /// No error has occured
        NoError,

        /// There was a network socket error
        SocketError,

        /// The connection timed out
        TimeoutError,

        /// There was an error communicating with the server
        ProtocolError,

        /// There was a problem with the remote host key
        HostKeyError,

        /// We failed to read or parse the key file used for authentication
        KeyFileError,

        /// We failed to authenticate
        AuthenticationError,

        /// The server closed our connection
        ClosedByServerError,

        /// The ssh-agent used for authenticating failed somehow
        AgentError,

        /// Something bad happened on the server
        InternalError
    };

    Q_ENUM(ErrorCode)

    explicit SshClient(QObject *parent = nullptr);
    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;

    void setConnectionListener(QObject* listener);
    void addAction(SshAction *action);
    virtual void init() = 0;
    virtual void resume() = 0;

    void setHost(QString host);
    void setCredentials(SshClientCredentials* creds);

protected:
    virtual void processAction() = 0;

signals:
    void transferFinished();
    void connectionFailed(SshClient::ErrorCode err);

public slots:

protected:
    QString _host;
    SshClientCredentials *_creds;
    QObject* _connectionListener;
    QQueue<SshAction*> m_actionQueue;
    SshAction *_currentAction;
    bool _connected = false;
    bool _waitingForConnection = false;
    ErrorCode _currentErrorCode = ErrorCode::NoError;
};


}

#endif // SSHCLIENT_H
