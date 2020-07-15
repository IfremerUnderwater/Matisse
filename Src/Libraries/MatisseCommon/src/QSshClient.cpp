#include "QSshClient.h"

#include <QFileInfo>

namespace MatisseCommon 
{

QSshClient::QSshClient(QObject* parent) : SshClient(parent) {}

void QSshClient::connectToHost() {
  qDebug() << tr("QSsh Connecting to host %1 as %2 ...")
                  .arg(m_host)
                  .arg(m_creds->username());
}

void QSshClient::disconnectFromHost() {
  qDebug() << QString("QSsh Disconnecting from host %1 ...").arg(m_host);
}

void QSshClient::upload(QString localPath, QString remotePath,
                        bool isDirUpload) {
  qDebug() << tr("QSshClient: Uploading file %1 to %2 ...")
                  .arg(localPath)
                  .arg(remotePath);

  QSsh::SftpJobId job;

  if (isDirUpload) {
    job = m_channel->uploadDir(localPath, remotePath);
  } else {
    job = m_channel->uploadFile(localPath, remotePath,
                                QSsh::SftpOverwriteExisting);
  }

  if (job != QSsh::SftpInvalidJob) {
    qDebug() << "QSshClient: Starting job #" << job;
  } else {
    qCritical() << "QSshClient: Invalid Job";
  }
}

void QSshClient::init() {}

void QSshClient::resume() {
  /* Try to reconnect after failed login */
  connectToRemoteHost();
}

void QSshClient::processAction() {
  if (!m_connected) {
    if (!m_waiting_for_connection) {
      connectToRemoteHost();
    }
    return;
  }

  if (m_action_queue.isEmpty()) {
    qWarning() << "QSshClient: SSH action queue empty, no action to process";
    return;
  }

  /* Nominal case : free previous action instance */
  if (m_current_action) {
    delete m_current_action;
  }

  m_current_action = m_action_queue.dequeue();
  m_current_action->init();
}

void QSshClient::connectToRemoteHost() {
  qDebug() << QString("QSshClient: Connecting to host %1 as %2 ...")
                  .arg(m_host)
                  .arg(m_creds->username());

  m_waiting_for_connection = true;

  QSsh::SshConnectionParameters params;
  params.setHost(m_host);
  params.setUserName(m_creds->username());
  params.setPassword(m_creds->password());
  params.authenticationType = QSsh::SshConnectionParameters::
      AuthenticationTypeTryAllPasswordBasedMethods;
  params.timeout = 30;
  params.setPort(22);

  m_connection = new QSsh::SshConnection(params, this);

  connect(m_connection, SIGNAL(connected()), SLOT(onConnected()));
  connect(m_connection, SIGNAL(error(QSsh::SshError)),
          SLOT(onConnectionError(QSsh::SshError)));
  connect(m_connection, SIGNAL(disconnected()), SLOT(onDisconnected()));

  m_connection->connectToHost();
}

void QSshClient::createSftpChannel() {
  qDebug() << "QSshClient: Creating SFTP channel...";

  m_channel = m_connection->createSftpChannel();

  if (!m_channel) {
    qCritical() << "QSshClient: Unexpected error null channel";
    return;
  }

  connect(m_channel.data(), SIGNAL(initialized()),
          SLOT(onChannelInitialized()));
  connect(m_channel.data(), SIGNAL(channelError(const QString&)),
          SLOT(onChannelError(const QString&)));
  connect(m_channel.data(),
          SIGNAL(finished(QSsh::SftpJobId, const SftpError, const QString&)),
          SLOT(onOpfinished(QSsh::SftpJobId, const SftpError, const QString&)));
  connect(m_channel.data(), SIGNAL(closed()), SLOT(onChannelClosed()));
  connect(m_channel.data(),
          SIGNAL(transferProgress(QSsh::SftpJobId, quint64, quint64)),
          SLOT(onTransferProgress(QSsh::SftpJobId, quint64, quint64)));

  m_channel->initialize();
}

void QSshClient::createRemoteShell(QString& command) {
  qDebug() << "QSshClient: Creating remote shell...";

  m_shell = m_connection->createRemoteShell();

  if (!m_shell) {
    qCritical() << "QSshClient: Unexpected error null shell";
    return;
  }

  m_shell_command = command;

  connect(m_shell.data(), SIGNAL(started()), SLOT(onShellStarted()));
  connect(m_shell.data(), SIGNAL(readyReadStandardOutput()),
          SLOT(onReadyReadStandardOutput()));
  connect(m_shell.data(), SIGNAL(readyReadStandardError()),
          SLOT(onReadyReadStandardError()));
  connect(m_shell.data(), SIGNAL(closed(int)), SLOT(onShellClosed(int)));

  m_shell->start();
}

void QSshClient::closeRemoteShell() 
{
  if (!m_shell) 
  {
    qCritical() << "QSshClient: trying to close shell, but shell is null";
    return;
  }

  /* Disconnect all signals but shell closing so that no further output/error is received */
  disconnect(this, SLOT(onShellStarted()));
  disconnect(this, SLOT(onReadyReadStandardOutput()));
  disconnect(this, SLOT(onReadyReadStandardError()));

  m_shell->close(); 
}

// UNUSED
void QSshClient::createRemoteProcess(QString& command) {
  qDebug() << QString("QSshClient: Creating remote process for command %1 ...")
                  .arg(command);

  m_shell = m_connection->createRemoteProcess(command.toLatin1());

  if (!m_shell) {
    qCritical() << "QSshClient: Unexpected error null shell";
    return;
  }

  connect(m_shell.data(), SIGNAL(started()), SLOT(onShellStarted()));
  connect(m_shell.data(), SIGNAL(readyReadStandardOutput()),
          SLOT(onReadyReadStandardOutput()));
  connect(m_shell.data(), SIGNAL(readyReadStandardError()),
          SLOT(onReadyReadStandardError()));
  connect(m_shell.data(), SIGNAL(closed(int)), SLOT(onShellClosed(int)));

  m_shell->start();
}

void QSshClient::executeCommand() {
  QString commandAndNl = m_shell_command.append("\n");

  qDebug()
      << QString("QSshClient: remote shell send command %1").arg(commandAndNl);

  m_shell->write(commandAndNl.toLatin1());
}

void QSshClient::uploadDir(QString localDir, QString remoteBaseDir) {
  QFileInfo info(localDir);

  if (!info.exists()) {
    qCritical()
        << QString("QSshClient: %1 cannot be uploaded : file does not exist")
               .arg(localDir);
    return;
  }

  m_local_path = info.canonicalFilePath();
  m_remote_path = remoteBaseDir;
  m_is_dir_upload = true;

  connectToRemoteHost();
}

void QSshClient::onConnected() {
  qDebug() << "QSshClient: Connected";

  m_connected = true;
  m_waiting_for_connection = false;

  if (!m_current_action) {
    processAction();
  }
}

void QSshClient::onDisconnected() {
  qDebug() << "QSshClient: disconnected";

  clearConnectionAndActionQueue();

  //_currentErrorCode = SshClient::ErrorCode::ClosedByServerError;
  // emit connectionFailed(_currentErrorCode);
}

void QSshClient::onConnectionError(QSsh::SshError err) {
  qCritical() << "QSshClient: Connection error" << err;

  mapError(err);

  m_waiting_for_connection = false;

  /* In case of authentication error, prompt for new login
  and give a chance to resume actions. Otherwise clear all */
  if (m_current_error_code != ErrorCode::AuthenticationError) {
    clearConnectionAndActionQueue();
  }

  emit si_connectionFailed(m_current_error_code);
}

void QSshClient::clearConnectionAndActionQueue() {
  m_connected = false;
  m_waiting_for_connection = false;

  disconnect(this, SLOT(onConnected()));
  disconnect(this, SLOT(onConnectionError(QSsh::SshError)));
  disconnect(this, SLOT(onDisconnected()));

  if (!m_action_queue.isEmpty()) {
    qCritical() << QString(
                       "QSshClient: Disconnected while %1 actions are still "
                       "pending in action queue, clearing queue...")
                       .arg(m_action_queue.count());
    m_action_queue.clear();
  }

  /* free last action instance */
  if (m_current_action) {
    delete m_current_action;
    m_current_action = NULL;
  }

  delete m_connection;
  m_connection = NULL;
}

void QSshClient::onChannelInitialized() {
  qDebug() << "QSshClient: Channel Initialized";

  m_current_action->execute();
}

void QSshClient::onChannelError(const QString& err) {
  qCritical() << "QSshClient: Error: " << err;
}

void QSshClient::onChannelClosed() {
  qDebug() << "QSshClient: Channel closed";
  disconnect(this, SLOT(onChannelInitialized()));
  disconnect(this, SLOT(onChannelError(const QString&)));
  disconnect(this, SLOT(onOpfinished(QSsh::SftpJobId, const SftpError,
                                     const QString&)));
  disconnect(this, SLOT(onChannelClosed()));
  disconnect(this, SLOT(onTransferProgress(QSsh::SftpJobId, quint64, quint64)));

  m_channel = NULL;

  if (m_action_queue.isEmpty()) {
    qDebug() << QString("QSshClient: Disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
  } else {
    /* If actions are still pending, start next action */
    processAction();
  }
}

void QSshClient::onOpfinished(QSsh::SftpJobId job, const SftpError errorType,
                              const QString& err) {
  qDebug() << "QSshClient: Finished job #" << job << ":"
           << (err.isEmpty() ? QStringLiteral("OK") : err);

  // notify manager
  emit si_transferFinished();

  qDebug() << "QSshClient: Closing channel...";
  m_channel->closeChannel();
}

void QSshClient::onTransferProgress(QSsh::SftpJobId job, quint64 progress,
                                    quint64 total) {
  qDebug() << QString("QSshClient: Upload job %1 progress %2 out of %3 bytes")
                  .arg(job)
                  .arg(progress)
                  .arg(total);
}

void QSshClient::onShellStarted() {
  qDebug() << "QSshClient: Shell started";

  m_current_action->execute();
}

void QSshClient::onReadyReadStandardOutput() {
  qDebug() << "QSshClient: ready read standard output";

  if (!m_current_action || m_current_action->isTerminated()) 
  {
    qDebug() << "Command action already terminated, ignoring output...";
    return;
  }

  if (!m_shell->isRunning()) 
  {
    qCritical() << "Shell not running properly, ignoring output...";
    return;
  }

  QByteArray outputStream = m_shell->readAllStandardOutput();
  emit si_shellOutputReceived(m_current_action, outputStream);
}

void QSshClient::onReadyReadStandardError() {
  qDebug() << "QSshClient: ready read standard error";

  if (!m_current_action || m_current_action->isTerminated()) {
    qWarning() << "Command action already terminated, ignoring error...";
    return;
  }

  if (!m_shell->isRunning()) 
  {
    qCritical() << "Shell not running properly, ignoring error...";
    return;
  }

  QByteArray errorStream = m_shell->readAllStandardError();
  emit si_shellErrorReceived(m_current_action, errorStream);
}

void QSshClient::onShellClosed(int exitStatus) {
  qDebug() << "QSshClient: Shell closed";

  disconnect(this, SLOT(onShellClosed(int)));

  m_shell = NULL;

  if (m_action_queue.isEmpty()) {
    qDebug() << QString("QSshClient: Disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
  } else {
    /* If actions are still pending, start next action */
    processAction();
  }
}

void QSshClient::mapError(QSsh::SshError err) {
  switch (err) {
    case QSsh::SshError::SshNoError:
      m_current_error_code = ErrorCode::NoError;
      break;

    case QSsh::SshError::SshSocketError:
      m_current_error_code = ErrorCode::SocketError;
      break;

    case QSsh::SshError::SshTimeoutError:
      m_current_error_code = ErrorCode::TimeoutError;
      break;

    case QSsh::SshError::SshProtocolError:
      m_current_error_code = ErrorCode::ProtocolError;
      break;

    case QSsh::SshError::SshHostKeyError:
      m_current_error_code = ErrorCode::HostKeyError;
      break;

    case QSsh::SshError::SshKeyFileError:
      m_current_error_code = ErrorCode::KeyFileError;
      break;

    case QSsh::SshError::SshAuthenticationError:
      m_current_error_code = ErrorCode::AuthenticationError;
      break;

    case QSsh::SshError::SshClosedByServerError:
      m_current_error_code = ErrorCode::ClosedByServerError;
      break;

    case QSsh::SshError::SshAgentError:
      m_current_error_code = ErrorCode::AgentError;
      break;

    case QSsh::SshError::SshInternalError:
      m_current_error_code = ErrorCode::InternalError;
      break;
  }

  qDebug() << QString("SSH error occurred : ") << m_current_error_code;
}

} // namespace MatisseCommon



