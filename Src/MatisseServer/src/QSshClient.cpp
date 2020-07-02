#include "QSshClient.h"
#include <QFileInfo>

using namespace MatisseServer;

QSshClient::QSshClient(QString host, SshClientCredentials* creds, QObject* parent) :
    SshClient(host, creds, parent)
{

}

void QSshClient::connectToHost()
{
    qDebug() << tr("QSsh Connecting to host %1 as %2 ...").arg(_host).arg(_creds->username());
}

void QSshClient::disconnectFromHost()
{
    qDebug() << QString("QSsh Disconnecting from host %1 ...").arg(_host);
}


//void QSshClient::upload(QString localFilePath, QString remotePath)
//{
//    QFileInfo info(localFilePath);
//
//    if (!info.exists()) {
//        qCritical() << QString("QSshClient: %1 cannot be uploaded : file does not exist").arg(localFilePath);
//        return;
//    }
//
//    m_localPath = info.canonicalFilePath();
//    m_remotePath = remotePath + QLatin1Char('/') + info.fileName();
//    m_isDirUpload = false;
//    
//    connectToRemoteHost();
//}

void QSshClient::upload(QString localPath, QString remotePath, bool isDirUpload) {
    qDebug() << tr("QSshClient: Uploading file %1 to %2 ...").arg(localPath).arg(remotePath);

    QSsh::SftpJobId job;

    if (isDirUpload) {
        job = m_channel->uploadDir(localPath, remotePath);
    } else {
        job = m_channel->uploadFile(localPath, remotePath,
            QSsh::SftpOverwriteExisting);
    }

    if (job != QSsh::SftpInvalidJob) {
        qDebug() << "QSshClient: Starting job #" << job;
    }
    else {
        qCritical() << "QSshClient: Invalid Job";
    }
}

void QSshClient::init()
{
    
}

void MatisseServer::QSshClient::resume()
{
    /* Try to reconnect after failed login */
    connectToRemoteHost();
}

void QSshClient::processAction()
{
    if (!_connected) {
        if (!_waitingForConnection) {
            connectToRemoteHost();
        }
        return;
    }

    if (m_actionQueue.isEmpty()) {
        qWarning() << "QSshClient: SSH action queue empty, no action to process";
        return;
    }

    ///* Case resuming failed action */
    //if (m_isActionPending) {
    //    if (!_currentAction) {
    //        qCritical() << "QSshClient: unexpected state, no current action while trying to resume after action failed";
    //        return;
    //    }

    //    qDebug() << "Resuming action " << _currentAction->type();
    //    m_isActionPending = false;
    //    _currentAction->init();
    //    return;
    //}
    
    /* Nominal case : free previous action instance */
    if (_currentAction) {
        delete _currentAction;
    }

    _currentAction = m_actionQueue.dequeue();
    _currentAction->init();
}

void QSshClient::connectToRemoteHost()
{
    qDebug() << QString("QSshClient: Connecting to host %1 as %2 ...").arg(_host).arg(_creds->username());

    _waitingForConnection = true;

    QSsh::SshConnectionParameters params;
    params.setHost(_host);
    params.setUserName(_creds->username());
    params.setPassword(_creds->password());
    params.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypeTryAllPasswordBasedMethods;
    params.timeout = 30;
    params.setPort(22);

    m_connection = new QSsh::SshConnection(params, this);

    connect(m_connection, SIGNAL(connected()), SLOT(onConnected()));
    connect(m_connection, SIGNAL(error(QSsh::SshError)), SLOT(onConnectionError(QSsh::SshError)));
    connect(m_connection, SIGNAL(disconnected()), SLOT(onDisconnected()));

    m_connection->connectToHost();
}

void QSshClient::createSftpChannel()
{
    qDebug() << "QSshClient: Creating SFTP channel...";

    m_channel = m_connection->createSftpChannel();

    if (!m_channel) {
        qCritical() << "QSshClient: Unexpected error null channel";
        return;
    }

    connect(m_channel.data(), SIGNAL(initialized()), SLOT(onChannelInitialized()));
    connect(m_channel.data(), SIGNAL(channelError(const QString&)), SLOT(onChannelError(const QString&)));
    connect(m_channel.data(), SIGNAL(finished(QSsh::SftpJobId, const SftpError, const QString&)),
        SLOT(onOpfinished(QSsh::SftpJobId, const SftpError, const QString&)));
    connect(m_channel.data(), SIGNAL(closed()), SLOT(onChannelClosed()));
    connect(m_channel.data(), SIGNAL(transferProgress(QSsh::SftpJobId, quint64, quint64)), SLOT(onTransferProgress(QSsh::SftpJobId, quint64, quint64)));

    m_channel->initialize();
}

void QSshClient::createRemoteShell(QString& command)
{
    qDebug() << "QSshClient: Creating remote shell...";

    m_shell = m_connection->createRemoteShell();

    if (!m_shell) {
        qCritical() << "QSshClient: Unexpected error null shell";
        return;
    }

    m_shellCommand = command;

    connect(m_shell.data(), SIGNAL(started()), SLOT(onShellStarted()));
    connect(m_shell.data(), SIGNAL(readyReadStandardOutput()), SLOT(onReadyReadStandardOutput()));
    connect(m_shell.data(), SIGNAL(readyReadStandardError()), SLOT(onReadyReadStandardError()));
    connect(m_shell.data(), SIGNAL(closed(int)), SLOT(onShellClosed(int))); 
    
    m_shell->start();
}

// UNUSED
void QSshClient::createRemoteProcess(QString& command)
{
    qDebug() << QString("QSshClient: Creating remote process for command %1 ...").arg(command);

    m_shell = m_connection->createRemoteProcess(command.toLatin1());

    if (!m_shell) {
        qCritical() << "QSshClient: Unexpected error null shell";
        return;
    }

    connect(m_shell.data(), SIGNAL(started()), SLOT(onShellStarted()));
    connect(m_shell.data(), SIGNAL(readyReadStandardOutput()), SLOT(onReadyReadStandardOutput()));
    connect(m_shell.data(), SIGNAL(readyReadStandardError()), SLOT(onReadyReadStandardError()));
    connect(m_shell.data(), SIGNAL(closed(int)), SLOT(onShellClosed(int)));

    m_shell->start();
}

void QSshClient::executeCommand()
{
    QString commandAndNl = m_shellCommand.append("\n");

    qDebug() << QString("QSshClient: remote shell send command %1").arg(commandAndNl);

    m_shell->write(commandAndNl.toLatin1());
}

void QSshClient::uploadDir(QString localDir, QString remoteBaseDir)
{
    QFileInfo info(localDir);

    if (!info.exists()) {
        qCritical() << QString("QSshClient: %1 cannot be uploaded : file does not exist").arg(localDir);
        return;
    }

    m_localPath = info.canonicalFilePath();
    m_remotePath = remoteBaseDir;
    m_isDirUpload = true;

    connectToRemoteHost();
}

void QSshClient::onConnected()
{
    qDebug() << "QSshClient: Connected";

    _connected = true;
    _waitingForConnection = false;

    if (!_currentAction) {
        processAction();
    }

}

void QSshClient::onDisconnected()
{
    qDebug() << "QSshClient: disconnected";

    clearConnectionAndActionQueue();

    //_currentErrorCode = SshClient::ErrorCode::ClosedByServerError;
    //emit connectionFailed(_currentErrorCode);
}


void QSshClient::onConnectionError(QSsh::SshError err)
{
    qCritical() << "QSshClient: Connection error" << err;

    mapError(err);

    _waitingForConnection = false;

    /* In case of authentication error, prompt for new login 
    and give a chance to resume actions. Otherwise clear all */
    if (_currentErrorCode != ErrorCode::AuthenticationError) {        
        clearConnectionAndActionQueue();
    }

    emit connectionFailed(_currentErrorCode);
}

void QSshClient::clearConnectionAndActionQueue()
{
    _connected = false;
    _waitingForConnection = false;

    disconnect(this, SLOT(onConnected()));
    disconnect(this, SLOT(onConnectionError(QSsh::SshError)));
    disconnect(this, SLOT(onDisconnected()));

    if (!m_actionQueue.isEmpty()) {
        qCritical() << QString("QSshClient: Disconnected while %1 actions are still pending in action queue, clearing queue...").arg(m_actionQueue.count());
        m_actionQueue.clear();
    }

    /* free last action instance */
    if (_currentAction) {
        delete _currentAction;
        _currentAction = NULL;
    }

    delete m_connection;
    m_connection = NULL;
}

void QSshClient::onChannelInitialized()
{
    qDebug() << "QSshClient: Channel Initialized";

    _currentAction->execute();
}

void QSshClient::onChannelError(const QString& err)
{
    qCritical() << "QSshClient: Error: " << err;
}

void QSshClient::onChannelClosed()
{
    qDebug() << "QSshClient: Channel closed";
    disconnect(this, SLOT(onChannelInitialized()));
    disconnect(this, SLOT(onChannelError(const QString&)));
    disconnect(this, SLOT(onOpfinished(QSsh::SftpJobId, const SftpError, const QString&)));
    disconnect(this, SLOT(onChannelClosed()));
    disconnect(this, SLOT(onTransferProgress(QSsh::SftpJobId, quint64, quint64)));
 
    m_channel = NULL;

    if (m_actionQueue.isEmpty()) {
        qDebug() << QString("QSshClient: Disconnecting from host %1...").arg(_host);
        m_connection->disconnectFromHost();
    }
    else {
        /* If actions are still pending, start next action */
        processAction();
    }

}

void QSshClient::onOpfinished(QSsh::SftpJobId job, const SftpError errorType, const QString& err)
{
    qDebug() << "QSshClient: Finished job #" << job << ":" << (err.isEmpty() ? QStringLiteral("OK") : err);
    
    // notify manager
    emit transferFinished();

    qDebug() << "QSshClient: Closing channel...";
    m_channel->closeChannel();
}

void QSshClient::onTransferProgress(QSsh::SftpJobId job, quint64 progress, quint64 total)
{
    qDebug() << QString("QSshClient: Upload progress %1 out of %2 bytes").arg(progress).arg(total);
    
}

void QSshClient::onShellStarted()
{
    qDebug() << "QSshClient: Shell started";

    _currentAction->execute();
}

void QSshClient::onReadyReadStandardOutput()
{
    qDebug() << "QSshClient: ready read standard output";

    QByteArray outputStream = m_shell->readAllStandardOutput();

    qDebug() << outputStream;
}

void QSshClient::onReadyReadStandardError()
{
    qDebug() << "QSshClient: ready read standard error";

    QByteArray errorStream = m_shell->readAllStandardError();

    qDebug() << errorStream;
}

void QSshClient::onShellClosed(int exitStatus)
{
    qDebug() << "QSshClient: Shell closed";

    disconnect(this, SLOT(onShellStarted()));
    disconnect(this, SLOT(onReadyReadStandardOutput()));
    disconnect(this, SLOT(onReadyReadStandardError()));
    disconnect(this, SLOT(onShellClosed(int exitStatus)));
}

void QSshClient::mapError(QSsh::SshError err)
{
    switch (err) {
    case QSsh::SshError::SshNoError:
        _currentErrorCode = ErrorCode::NoError;
        break;

    case QSsh::SshError::SshSocketError:
        _currentErrorCode = ErrorCode::SocketError;
        break;

    case QSsh::SshError::SshTimeoutError:
        _currentErrorCode = ErrorCode::TimeoutError;
        break;

    case QSsh::SshError::SshProtocolError:
        _currentErrorCode = ErrorCode::ProtocolError;
        break;

    case QSsh::SshError::SshHostKeyError:
        _currentErrorCode = ErrorCode::HostKeyError;
        break;

    case QSsh::SshError::SshKeyFileError:
        _currentErrorCode = ErrorCode::KeyFileError;
        break;

    case QSsh::SshError::SshAuthenticationError:
        _currentErrorCode = ErrorCode::AuthenticationError;
        break;

    case QSsh::SshError::SshClosedByServerError:
        _currentErrorCode = ErrorCode::ClosedByServerError;
        break;

    case QSsh::SshError::SshAgentError:
        _currentErrorCode = ErrorCode::AgentError;
        break;

    case QSsh::SshError::SshInternalError:
        _currentErrorCode = ErrorCode::InternalError;
        break;
    }

    qDebug() << QString("SSH error occurred : ") << _currentErrorCode;
}





