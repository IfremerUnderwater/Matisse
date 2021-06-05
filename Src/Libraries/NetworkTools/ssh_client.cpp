#include "ssh_client.h"

namespace network_tools {

SshClient::SshClient() :
    NetworkCommandClient()
{

}

void SshClient::init()
{

}

void SshClient::sl_createRemoteShell(QString& _command)
{
    qDebug() << "SSH Client: Creating remote shell...";

    m_shell = m_connection->createRemoteShell();

    if (!m_shell) {
      qCritical() << "SSH Client: Unexpected error null shell";
      return;
    }

    m_shell_command = _command;

    connect(m_shell.data(), SIGNAL(started()), SLOT(sl_onShellStarted()));
    connect(m_shell.data(), SIGNAL(readyReadStandardOutput()),
            SLOT(sl_onReadyReadStandardOutput()));
    connect(m_shell.data(), SIGNAL(readyReadStandardError()),
            SLOT(sl_onReadyReadStandardError()));
    connect(m_shell.data(), SIGNAL(closed(int)), SLOT(sl_onShellClosed(int)));

    m_shell->start();

    /* Signal 10% progress for shell init (remote command process) */
    emit si_progressUpdate(10);
    m_last_signalled_progress = 10;
}

void SshClient::sl_closeRemoteShell()
{
    if (!m_shell)
    {
      qCritical() << "SSH Client: trying to close shell, but shell is null";
      return;
    }

    /* Disconnect all signals but shell closing so that no further output/error is received */
    disconnect(this, SLOT(sl_onShellStarted()));
    disconnect(this, SLOT(sl_onReadyReadStandardOutput()));
    disconnect(this, SLOT(sl_onReadyReadStandardError()));

    m_shell->close();
}

void SshClient::sl_executeCommand()
{
    QString command_and_nl = m_shell_command.append("\n");

    qDebug()
        << QString("SSH Client: remote shell send command %1").arg(command_and_nl);

    m_shell->write(command_and_nl.toLatin1());
}

void SshClient::sl_onShellStarted() {
  qDebug() << "SSH Client: Shell started";

  /* Signal 30% progress on shell established */
  emit si_progressUpdate(30);
  m_last_signalled_progress = 30;

  m_current_action->execute();
}

void SshClient::sl_onReadyReadStandardOutput() {
  qDebug() << "SSH Client: ready read standard output";

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

  QByteArray output_stream = m_shell->readAllStandardOutput();
  emit si_shellOutputReceived(m_current_action, output_stream);

  /* Increment from 60% progress on shell established */
  int new_progress = 0;

  if (m_last_signalled_progress < 60) {
    new_progress = 60;
  } else {
    if (m_last_signalled_progress < 90) {
      new_progress += 10;
    }
  }

  if (new_progress > 0) {
    emit si_progressUpdate(new_progress);
    m_last_signalled_progress = new_progress;
  }
}

void SshClient::sl_onReadyReadStandardError() {
  qDebug() << "SSH Client: ready read standard error";

  if (!m_current_action || m_current_action->isTerminated()) {
    qWarning() << "Command action already terminated, ignoring error...";
    return;
  }

  if (!m_shell->isRunning())
  {
    qCritical() << "Shell not running properly, ignoring error...";
    return;
  }

  QByteArray error_stream = m_shell->readAllStandardError();
  emit si_shellErrorReceived(m_current_action, error_stream);
}

void SshClient::sl_onShellClosed(int _exit_status) {
  qDebug() << QString("SSH Client: Shell closed with exit status %1").arg(_exit_status);

  disconnect(this, SLOT(sl_onShellClosed(int)));

  m_shell = NULL;

  if (m_action_queue.isEmpty()) {
    qDebug() << QString("SSH Client: Disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
  } else {
    /* If actions are still pending, start next action */
    processAction();
  }
}


/* TODO mutualize code in a connection wrapper class */

void SshClient::resetConnection() {
  if (m_connected) {
    qDebug() << "SSH Client: Closing SSH/SFTP connection...";
    m_connection->disconnectFromHost();
    m_connected = false;
    m_waiting_for_connection = false;
    m_obsolete_connections.insert(m_connection); /* keep track of connection object until disconnected */
    m_connection = NULL;

    clearConnectionAndActionQueue();

  } else {
    qDebug() << "SSH Client: SSH/SFTP gateway not connected, ready to reconnect";
  }
}

void SshClient::clearActions() {
  clearConnectionAndActionQueue();
}

void SshClient::resume() {
  /* Try to reconnect after failed login */
  connectToRemoteHost();
}

void SshClient::processAction() {
  if (!m_connected) {
    if (!m_waiting_for_connection) {
      connectToRemoteHost();
    }
    return;
  }

  if (m_action_queue.isEmpty()) {
    qWarning() << "SSH Client: SSH action queue empty, no action to process";
    return;
  }


  /* Nominal case : free previous action instance */
  if (m_current_action) {
      disconnectAction(m_current_action);
      delete m_current_action;
  }

//  /* Clearing dir contents buffer if previous operation was listing dir contents (and buffer was filled) */
//  if (!m_dir_contents_buffer.isEmpty()) {
//    qDebug() << "SFTP Client: Clearing dir contents buffer for previous operation...";
//    m_dir_contents_received = false;
//    qDeleteAll(m_dir_contents_buffer);
//    m_dir_contents_buffer.clear();
//  }

  m_current_action = m_action_queue.dequeue();
  connectAction(m_current_action);

  qDebug() << "SSH Client: Processing SSH action of type " << m_current_action->type();
  m_current_action->init();
}

void SshClient::connectToRemoteHost() {
  qDebug() << QString("SSH Client: Connecting to host %1 as %2 ...")
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

  connect(m_connection, SIGNAL(connected()), SLOT(sl_onConnected()));
  connect(m_connection, SIGNAL(error(QSsh::SshError)),
          SLOT(sl_onConnectionError(QSsh::SshError)));
  connect(m_connection, SIGNAL(disconnected()), SLOT(sl_onDisconnected()));

  m_connection->connectToHost();
}

void SshClient::clearConnectionAndActionQueue() {
  m_connected = false;
  m_waiting_for_connection = false;

  disconnect(this, SLOT(sl_onConnected()));
  disconnect(this, SLOT(sl_onConnectionError(QSsh::SshError)));
  disconnect(this, SLOT(sl_onDisconnected()));

  if (!m_action_queue.isEmpty()) {
    qCritical() << QString(
                       "SSH Client: Disconnected while %1 actions are still "
                       "pending in action queue, clearing queue...")
                       .arg(m_action_queue.count());
    qDeleteAll(m_action_queue);
    m_action_queue.clear();
  }

  /* free last action instance */
  if (m_current_action) {
      disconnectAction(m_current_action);
      delete m_current_action;
      m_current_action = NULL;
  }

  /* free connection except if it was tracked as obsolete (connection reset by calling agent) */
  if (m_connection) {
    delete m_connection;
    m_connection = NULL;
  }
}

void SshClient::mapConnectionError(QSsh::SshError _err) {
  switch (_err) {
    case QSsh::SshError::SshNoError:
      m_current_cx_error = eConnectionError::NO_ERROR;
      break;

    case QSsh::SshError::SshSocketError:
      m_current_cx_error = eConnectionError::SOCKET_ERROR;
      break;

    case QSsh::SshError::SshTimeoutError:
      m_current_cx_error = eConnectionError::TIMEOUT_ERROR;
      break;

    case QSsh::SshError::SshProtocolError:
      m_current_cx_error = eConnectionError::PROTOCOL_ERROR;
      break;

    case QSsh::SshError::SshHostKeyError:
      m_current_cx_error = eConnectionError::HOST_KEY_ERROR;
      break;

    case QSsh::SshError::SshKeyFileError:
      m_current_cx_error = eConnectionError::KEY_FILE_ERROR;
      break;

    case QSsh::SshError::SshAuthenticationError:
      m_current_cx_error = eConnectionError::AUTHENTICATION_ERROR;
      break;

    case QSsh::SshError::SshClosedByServerError:
      m_current_cx_error = eConnectionError::CLOSED_BY_SERVER_ERROR;
      break;

    case QSsh::SshError::SshAgentError:
      m_current_cx_error = eConnectionError::AGENT_ERROR;
      break;

    case QSsh::SshError::SshInternalError:
      m_current_cx_error = eConnectionError::INTERNAL_ERROR;
      break;
  }

  qDebug() << QString("SSH connection error occurred : ") << m_current_cx_error;
}

void SshClient::sl_onConnected() {
  qDebug() << "SSH Client: Connected";

  m_connected = true;
  m_waiting_for_connection = false;

  if (!m_current_action) {
    processAction();
  }
}

void SshClient::sl_onDisconnected() {
  qDebug() << "SSH Client: disconnected";

  QObject* emitter = sender();
  QSsh::SshConnection* expired_connection = static_cast<QSsh::SshConnection*>(emitter);

  /* Case : the connection was closed by calling agent */
  if (m_obsolete_connections.contains(expired_connection)) {
    qDebug() << "SSH Client: clearing obsolete connection";

    disconnect(expired_connection, SIGNAL(connected()));
    disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
    disconnect(expired_connection, SIGNAL(disconnected()));

    m_obsolete_connections.remove(expired_connection);
    delete expired_connection;

    return;
  }

  /* Unconsistent case : the connection is neither the current connection, nor tracked as an ancient connection */
  if (expired_connection != m_connection) {
    qWarning() << "SSH Client: unknown connection object, clearing anyway";

    disconnect(expired_connection, SIGNAL(connected()));
    disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
    disconnect(expired_connection, SIGNAL(disconnected()));

    delete expired_connection;

    return;
  }

  /* Nominal case : the connection was closed after completing job */
  clearConnectionAndActionQueue();
}

void SshClient::sl_onConnectionError(QSsh::SshError _err) {
  qCritical() << "SSH Client: Connection error" << _err;

  mapConnectionError(_err);

  m_waiting_for_connection = false;

  /* In case of authentication error, prompt for new login
  and give a chance to resume actions. Otherwise clear all */
  if (m_current_cx_error != eConnectionError::AUTHENTICATION_ERROR) {
    clearConnectionAndActionQueue();
  }

  emit si_connectionFailed(m_current_cx_error);
}

} // namespace network_tools
