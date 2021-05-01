#include "sftp_client.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "FileUtils.h"

namespace MatisseCommon 
{

SftpClient::SftpClient(QObject* parent)
    : NetworkClient(parent),
  m_connection(NULL),
  m_obsolete_connections(),
  m_dir_contents_buffer(),
  m_dir_contents_received(false), 
  m_file_filters(),
  m_progress_matrix() 
{}

void SftpClient::upload(QString _local_path, QString _remote_path,
                        bool _is_dir_upload) {
  qDebug() << tr("QSshClient: Uploading file %1 to %2 ...")
                  .arg(_local_path)
                  .arg(_remote_path);

  /* Init progress indicators */
  quint32 file_count;
  quint64 transfer_size;

  if (_is_dir_upload) {
    transfer_size = FileUtils::dirSize(_local_path); 
    file_count = FileUtils::fileCount(_local_path);
  } else {
    QFileInfo file(_local_path); // Assume file exists (checked by calling action)
    transfer_size = file.size();
    file_count = 1;
  }

  reinitProgressIndicators(transfer_size, file_count);

  /* Starting appropriate task */
  QSsh::SftpJobId job;

  if (_is_dir_upload) {
    job = m_channel->uploadDir(_local_path, _remote_path);
  } else {
    job = m_channel->uploadFile(_local_path, _remote_path,
                                QSsh::SftpOverwriteExisting);
  }

  if (job != QSsh::SftpInvalidJob) {
    qDebug() << "QSshClient: Starting job #" << job;
  } else {
    qCritical() << "QSshClient: Invalid Job";
  }
}

void SftpClient::download(QString _remote_path, QString _local_path,
                          bool _is_dir_download) 
{
  if (_is_dir_download) {
    /* Start by listing source dir contents to enable progress tracking */
    /* Subdirs recursion not supported (will hang at 100% for a while if dir has subdirs) */
    dirContent(_remote_path, FileTypeFilter::Files);

  } else { // single file download
    qDebug() << tr("QSshClient: Downloading file %1 to %2 ...")
      .arg(_remote_path).arg(_local_path);

    reinitProgressIndicators(0, 1); /* Transfer size is not known yet */
    
    QSsh::SftpJobId job = m_channel->downloadFile(_remote_path, _local_path,
      QSsh::SftpOverwriteExisting);

    if (job != QSsh::SftpInvalidJob) {
      qDebug() << "QSshClient: Starting job #" << job;
    } else {
      qCritical() << "QSshClient: Invalid Job";
    }
  }

}

void SftpClient::dirContent(QString _remote_dir_path,
                            FileTypeFilters _flags,
                            QStringList _file_filters)
{
  qDebug() << tr("QSshClient: Listing content from dir %1 ...").arg(_remote_dir_path);

  m_file_type_flags = _flags;
  m_file_filters = _file_filters;

  QSsh::SftpJobId job;

  if (m_current_action->type() == NetworkAction::NetworkActionType::ListDirContent) {
    // Do not signal progress if called prior to dir downloading
    emit si_progressUpdate(10);
    m_last_signalled_progress = 10;
  }

  job = m_channel->listDirectory(_remote_dir_path);

  if (job != QSsh::SftpInvalidJob) {
    qDebug() << "QSshClient: Starting job #" << job;
  } else {
    qCritical() << "QSshClient: Invalid Job";
  }
}

void SftpClient::init() {}

void SftpClient::resetConnection() {
  if (m_connected) {
    qDebug() << "QSshClient: Closing SSH/SFTP connection...";
    m_connection->disconnectFromHost();
    m_connected = false;
    m_waiting_for_connection = false;
    m_obsolete_connections.insert(m_connection); /* keep track of connection object until disconnected */
    m_connection = NULL;

    clearConnectionAndActionQueue();

  } else {
    qDebug() << "QSshClient: SSH/SFTP gateway not connected, ready to reconnect";  
  }
}

void SftpClient::clearActions() {
  clearConnectionAndActionQueue();
}

void SftpClient::resume() {
  /* Try to reconnect after failed login */
  connectToRemoteHost();
}

void SftpClient::processAction() {
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

  /* Clearing dir contents buffer if previous operation was listing dir contents (and buffer was filled) */
  if (!m_dir_contents_buffer.isEmpty()) {
    qDebug() << "QSshClient: Clearing dir contents buffer for previous operation...";
    m_dir_contents_received = false;
    qDeleteAll(m_dir_contents_buffer);
    m_dir_contents_buffer.clear();
  }

  m_current_action = m_action_queue.dequeue();
  qDebug() << "QSshClient: Processing SSH action of type " << m_current_action->type();
  m_current_action->init();
}

void SftpClient::connectToRemoteHost() {
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

  connect(m_connection, SIGNAL(connected()), SLOT(sl_onConnected()));
  connect(m_connection, SIGNAL(error(QSsh::SshError)),
          SLOT(sl_onConnectionError(QSsh::SshError)));
  connect(m_connection, SIGNAL(disconnected()), SLOT(sl_onDisconnected()));

  m_connection->connectToHost();
}

void SftpClient::createSftpChannel() {
  qDebug() << "QSshClient: Creating SFTP channel...";

  m_channel = m_connection->createSftpChannel();

  if (!m_channel) {
    qCritical() << "QSshClient: Unexpected error null channel";
    return;
  }

  connect(m_channel.data(), SIGNAL(initialized()),
          SLOT(sl_onChannelInitialized()));
  connect(m_channel.data(), SIGNAL(channelError(const QString&)),
          SLOT(sl_onChannelError(const QString&)));
  connect(m_channel.data(),
          SIGNAL(finished(QSsh::SftpJobId, const SftpError, const QString&)),
          SLOT(sl_onOpfinished(QSsh::SftpJobId, const SftpError, const QString&)));
  connect(m_channel.data(), SIGNAL(closed()), SLOT(sl_onChannelClosed()));
  connect(m_channel.data(),
          SIGNAL(transferProgress(QSsh::SftpJobId, quint64, quint64)),
          SLOT(sl_onTransferProgress(QSsh::SftpJobId, quint64, quint64)));
  connect(m_channel.data(),
          SIGNAL(fileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>)),
          SLOT(sl_onFileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>)));

  m_channel->initialize();
}

void SftpClient::createRemoteShell(QString& command) {
  qDebug() << "QSshClient: Creating remote shell...";

  m_shell = m_connection->createRemoteShell();

  if (!m_shell) {
    qCritical() << "QSshClient: Unexpected error null shell";
    return;
  }

  m_shell_command = command;

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

void SftpClient::closeRemoteShell()
{
  if (!m_shell) 
  {
    qCritical() << "QSshClient: trying to close shell, but shell is null";
    return;
  }

  /* Disconnect all signals but shell closing so that no further output/error is received */
  disconnect(this, SLOT(sl_onShellStarted()));
  disconnect(this, SLOT(sl_onReadyReadStandardOutput()));
  disconnect(this, SLOT(sl_onReadyReadStandardError()));

  m_shell->close(); 
}


void SftpClient::executeCommand() {
  QString commandAndNl = m_shell_command.append("\n");

  qDebug()
      << QString("QSshClient: remote shell send command %1").arg(commandAndNl);

  m_shell->write(commandAndNl.toLatin1());
}

void SftpClient::startDownloadDir(QString _remote_path, QString _local_path)
{
  qDebug() << tr("QSshClient: Downloading dir %1 to %2 ...")
                  .arg(_remote_path)
                  .arg(_local_path);

  QSsh::SftpJobId job;

  job = m_channel->downloadDir(_remote_path, _local_path,
    QSsh::SftpOverwriteMode::SftpOverwriteExisting);

  if (job != QSsh::SftpInvalidJob) {
    qDebug() << "QSshClient: Starting job #" << job;
  } else {
    qCritical() << "QSshClient: Invalid Job";
  }
}

void SftpClient::reinitProgressIndicators(quint64 _transfer_size, quint32 _matrix_size) {
  m_current_transfer_size = _transfer_size; 
  m_total_received_bytes = 0;
  m_progress_matrix.clear();
  m_progress_matrix.resize(_matrix_size);
  m_progress_matrix.fill(0);  // initialize all cells to 0 received bytes
  m_last_signalled_progress = 0;
  m_progress_offset = 0;
}


void SftpClient::sl_onConnected() {
  qDebug() << "QSshClient: Connected";

  m_connected = true;
  m_waiting_for_connection = false;

  if (!m_current_action) {
    processAction();
  }
}

void SftpClient::sl_onDisconnected() {
  qDebug() << "QSshClient: disconnected";

  QObject* emitter = sender();
  SshConnection* expired_connection = static_cast<SshConnection*>(emitter);

  /* Case : the connection was closed by calling agent */
  if (m_obsolete_connections.contains(expired_connection)) {
    qDebug() << "QSshClient: clearing obsolete connection";

    disconnect(expired_connection, SIGNAL(connected()));
    disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
    disconnect(expired_connection, SIGNAL(disconnected()));

    m_obsolete_connections.remove(expired_connection);
    delete expired_connection;

    return;
  }

  /* Unconsistent case : the connection is neither the current connection, nor tracked as an ancient connection */
  if (expired_connection != m_connection) {
    qWarning() << "QSshClient: unknown connection object, clearing anyway";

    disconnect(expired_connection, SIGNAL(connected()));
    disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
    disconnect(expired_connection, SIGNAL(disconnected()));

    delete expired_connection;

    return;
  }

  /* Nominal case : the connection was closed after completing job */
  clearConnectionAndActionQueue();
}

void SftpClient::sl_onConnectionError(QSsh::SshError err) {
  qCritical() << "QSshClient: Connection error" << err;

  mapConnectionError(err);

  m_waiting_for_connection = false;

  /* In case of authentication error, prompt for new login
  and give a chance to resume actions. Otherwise clear all */
  if (m_current_cx_error != ConnectionError::AuthenticationError) {
    clearConnectionAndActionQueue();
  }

  emit si_connectionFailed(m_current_cx_error);
}

void SftpClient::clearConnectionAndActionQueue() {
  m_connected = false;
  m_waiting_for_connection = false;

  disconnect(this, SLOT(sl_onConnected()));
  disconnect(this, SLOT(sl_onConnectionError(QSsh::SshError)));
  disconnect(this, SLOT(sl_onDisconnected()));

  if (!m_action_queue.isEmpty()) {
    qCritical() << QString(
                       "QSshClient: Disconnected while %1 actions are still "
                       "pending in action queue, clearing queue...")
                       .arg(m_action_queue.count());
    qDeleteAll(m_action_queue);
    m_action_queue.clear();
  }

  /* free last action instance */
  if (m_current_action) {
    delete m_current_action;
    m_current_action = NULL;
  }

  /* free connection except if it was tracked as obsolete (connection reset by calling agent) */
  if (m_connection) {
    delete m_connection;
    m_connection = NULL;
  }
}

void SftpClient::sl_onChannelInitialized() {
  qDebug() << "QSshClient: Channel Initialized";

  m_current_action->execute();
}

void SftpClient::sl_onChannelError(const QString& err) {
  qCritical() << "QSshClient: Error: " << err;
}

void SftpClient::sl_onChannelClosed() {
  qDebug() << "QSshClient: Channel closed";
  disconnect(this, SLOT(sl_onChannelInitialized()));
  disconnect(this, SLOT(sl_onChannelError(const QString&)));
  disconnect(this, SLOT(sl_onOpfinished(QSsh::SftpJobId, const SftpError,
                                     const QString&)));
  disconnect(this, SLOT(sl_onChannelClosed()));
  disconnect(this, SLOT(sl_onTransferProgress(QSsh::SftpJobId, quint64, quint64)));
  disconnect(this, SLOT(sl_onFileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>)));

  m_channel = NULL;

  if (m_action_queue.isEmpty()) {
    qDebug() << QString("QSshClient: Disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
  } else {
    /* If actions are still pending, start next action */
    processAction();
  }
}

void SftpClient::sl_onOpfinished(QSsh::SftpJobId _job, const SftpError _error_type,
                              const QString& _err_msg) {
  
  if (_error_type != QSsh::SftpError::NoError) {
    qCritical()
        << QString("QSshClient: Job #%1 failed : %2").arg(_job).arg(_err_msg);

    mapTransferError(_error_type);  
    emit si_transferFailed(m_current_action, m_current_tx_error);
    return;
  }
  
  qDebug() << "QSshClient: Finished job #" << _job << ": OK";

  if (m_dir_contents_received) { // Case job for listing dir contents complete

    if (m_current_action->type() == NetworkAction::NetworkActionType::DownloadDir) {
      // Sub-case : job was started internally prior to downloading

      m_dir_contents_received = false; // uncheck to avoid looping on dir download 

      /* Init progress indicators */
      quint64 transfer_size = 0;
      quint32 file_count = m_dir_contents_buffer.size();

      for (NetworkFileInfo *sfi : m_dir_contents_buffer) {
        transfer_size += sfi->size();
      }

      reinitProgressIndicators(transfer_size, file_count);

      DownloadDirAction* dl_action = static_cast<DownloadDirAction*>(m_current_action);
      startDownloadDir(dl_action->remoteDir(), dl_action->localBaseDir());
      return; // do not close channel yet
    } 

    else { // nominal sub-case : ListDirContents action was explicitely called

      // notify manager
      qDebug() << "QSshClient: signalling dir contents...";

      /* Copy buffer (maybe empty if elements were filtered) */
      QList<NetworkFileInfo*> dir_contents(m_dir_contents_buffer);
      emit si_dirContents(dir_contents);
      /* Channel will be closed hereafter */
    } 

  } else { // Case : upload or download job complete

    // notify manager
    qDebug() << "QSshClient: signalling download or upload complete...";
    emit si_transferFinished(m_current_action);
  }

  qDebug() << "QSshClient: Closing channel...";
  m_channel->closeChannel();
}

void SftpClient::sl_onTransferProgress(QSsh::SftpJobId job, quint64 progress,
                                    quint64 total) {
  //qDebug() << QString("QSshClient: Upload job %1 progress %2 out of %3 bytes")
  //                .arg(job)
  //                .arg(progress)
  //                .arg(total);

  if (m_current_transfer_size == 0) {
    if (m_current_action->type() == NetworkAction::NetworkActionType::DownloadFile) {
      m_current_transfer_size = total; // total transfer size is being discovered with current file size
    } else {
      qCritical() << "QSshClient: current transfer size unknown (0), cannot signal progress";
      return; 
    }
  }

  if (m_progress_offset == 0) {
    /* Init offset with first job id */
    m_progress_offset = job;
  }

  if (m_last_signalled_progress == 100) {
    // already complete ==> skip
    return;
  }

  /* Compute progress increment in bytes */
  quint32 file_index = job - m_progress_offset;
  quint64 prev_progress = m_progress_matrix[file_index];
  quint64 increment = progress - prev_progress;  
  m_total_received_bytes += increment;

  /* Update matrix */
  m_progress_matrix[file_index] = progress;

  /* Notify client process */
  float progress_rate = (float)m_total_received_bytes / (float)m_current_transfer_size;
  float progress_percentage = progress_rate * 100.0f;
  //int rounded_progress = qRound(progress_percentage);
  int rounded_progress = (int)progress_percentage; // round to the lower bound to reduce hanging at 100%

  if (rounded_progress < m_last_signalled_progress) {
    qCritical() << QString(
                       "QsshClient: something went wrong while computing "
                       "progress : previous=%1 ; new=%2")
                       .arg(m_last_signalled_progress)
                       .arg(rounded_progress);
    return;
  }

  if (rounded_progress == m_last_signalled_progress) {
    // no visible progress : do not signal
    return;
  }

  m_last_signalled_progress = rounded_progress;
  emit si_progressUpdate(rounded_progress);
}

void SftpClient::sl_onShellStarted() {
  qDebug() << "QSshClient: Shell started";

  /* Signal 30% progress on shell established */
  emit si_progressUpdate(30);
  m_last_signalled_progress = 30;

  m_current_action->execute();
}

void SftpClient::sl_onReadyReadStandardOutput() {
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

void SftpClient::sl_onReadyReadStandardError() {
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

void SftpClient::sl_onShellClosed(int exitStatus) {
  qDebug() << "QSshClient: Shell closed";

  disconnect(this, SLOT(sl_onShellClosed(int)));

  m_shell = NULL;

  if (m_action_queue.isEmpty()) {
    qDebug() << QString("QSshClient: Disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
  } else {
    /* If actions are still pending, start next action */
    processAction();
  }
}

void SftpClient::sl_onFileInfoAvailable(
    QSsh::SftpJobId _job, const QList<QSsh::SftpFileInfo>& _file_info_list) 
{
  qDebug() << QString("QSshClient: Received %1 file info elements")
                  .arg(_file_info_list.count());

  m_dir_contents_received = true;

  bool keep_dirs = m_file_type_flags & FileTypeFilter::Dirs;
  bool keep_files = m_file_type_flags & FileTypeFilter::Files;

  for (QSsh::SftpFileInfo info : _file_info_list) {
    QString name = info.name;
    bool is_dir = false;

    switch (info.type)
    { 
    case QSsh::SftpFileType::FileTypeRegular :
      //qDebug() << "File " << name;
      break;
    case QSsh::SftpFileType::FileTypeDirectory:
      //qDebug() << "Dir " << name;
      is_dir = true;
      break;
    default:
      qWarning() << QString("Unexpected file type %1 for file %2")
                        .arg(info.type)
                        .arg(name);
      break;
    }

    /* Skipping dirs if flag not present */
    if (is_dir && !keep_dirs) {
      continue;
    }

    /* Skipping files if flag not present */
    if (!is_dir && !keep_files) {
      continue;
    }

    /* Skipping Dot and DotDot */
    if (is_dir) {
      if ((name == ".") || (name == "..")) {
        continue;
      }
    }

    /* Filter files */
    bool matches_filter = true;

    if (!is_dir) {
      for (QString filter : m_file_filters) {
        /* Assume filter is of type '*.ext' (checked by calling action) */
        QString file_suffix = filter.mid(1);
        if (name.endsWith(file_suffix)) {
          /* File matches current filter */
          matches_filter = true;
          break;
        } else {
          matches_filter = false;
        }
      }

      if (!matches_filter) {
        /* If filters are specified and file doesn't match any : skip */
        continue;
      }
    }

    /* Mapping last modification timestamp from UNIX date/time format */
    QDateTime last_modified = QDateTime::fromSecsSinceEpoch(info.mtime);

    quint64 size = info.size;
    if (!info.sizeValid) {
      size = 0;
    }

    NetworkFileInfo* ssh_fi = new NetworkFileInfo(name, is_dir, size, last_modified);
    m_dir_contents_buffer.append(ssh_fi);

  } // _file_info_list

  if (m_current_action->type() == NetworkAction::NetworkActionType::ListDirContent) {
    // Do not signal progress if called prior to dir downloading (current action DownloadDir)

    /* Increment from 50% */
    int new_progress = 0;

    if (m_last_signalled_progress < 50) {
      new_progress = 50;
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

}

void SftpClient::mapConnectionError(QSsh::SshError _err) {
  switch (_err) {
    case QSsh::SshError::SshNoError:
      m_current_cx_error = ConnectionError::NoError;
      break;

    case QSsh::SshError::SshSocketError:
      m_current_cx_error = ConnectionError::SocketError;
      break;

    case QSsh::SshError::SshTimeoutError:
      m_current_cx_error = ConnectionError::TimeoutError;
      break;

    case QSsh::SshError::SshProtocolError:
      m_current_cx_error = ConnectionError::ProtocolError;
      break;

    case QSsh::SshError::SshHostKeyError:
      m_current_cx_error = ConnectionError::HostKeyError;
      break;

    case QSsh::SshError::SshKeyFileError:
      m_current_cx_error = ConnectionError::KeyFileError;
      break;

    case QSsh::SshError::SshAuthenticationError:
      m_current_cx_error = ConnectionError::AuthenticationError;
      break;

    case QSsh::SshError::SshClosedByServerError:
      m_current_cx_error = ConnectionError::ClosedByServerError;
      break;

    case QSsh::SshError::SshAgentError:
      m_current_cx_error = ConnectionError::AgentError;
      break;

    case QSsh::SshError::SshInternalError:
      m_current_cx_error = ConnectionError::InternalError;
      break;
  }

  qDebug() << QString("SSH connection error occurred : ") << m_current_cx_error;
}

void SftpClient::mapTransferError(QSsh::SftpError _err)
{
  switch (_err) {
    case QSsh::SftpError::NoError:
      m_current_tx_error = TransferError::NoError;
      break;

    case QSsh::SftpError::EndOfFile:
      m_current_tx_error = TransferError::EndOfFile;
      break;

    case QSsh::SftpError::FileNotFound:
      m_current_tx_error = TransferError::FileNotFound;
      break;

    case QSsh::SftpError::PermissionDenied:
      m_current_tx_error = TransferError::PermissionDenied;
      break;

    case QSsh::SftpError::GenericFailure:
      m_current_tx_error = TransferError::GenericFailure;
      break;

    case QSsh::SftpError::BadMessage:
      m_current_tx_error = TransferError::BadMessage;
      break;

    case QSsh::SftpError::NoConnection:
      m_current_tx_error = TransferError::NoConnection;
      break;

    case QSsh::SftpError::ConnectionLost:
      m_current_tx_error = TransferError::ConnectionLost;
      break;

    case QSsh::SftpError::UnsupportedOperation:
      m_current_tx_error = TransferError::UnsupportedOperation;
      break;
  }

  qDebug() << QString("SFTP error occurred : ") << m_current_tx_error;
}

} // namespace MatisseCommon



