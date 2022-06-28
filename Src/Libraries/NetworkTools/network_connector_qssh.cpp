#include "network_connector_qssh.h"
#include "file_utils.h"

using namespace system_tools;

namespace network_tools {

NetworkConnectorQSsh::NetworkConnectorQSsh() :
    NetworkConnector(),
    m_connection(NULL),
    m_obsolete_connections()
{

}

void NetworkConnectorQSsh::sl_initFileChannel()
{
    qDebug() << "NetworkConnectorQSsh: Creating SFTP channel...";

    reinitBeforeFileOperation();

    m_channel = m_connection->createSftpChannel();

    if (!m_channel) {
        qCritical() << "NetworkConnectorQSsh: Unexpected error null channel";
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

void NetworkConnectorQSsh::reinitBeforeFileOperation() {
    m_download_dir_ongoing = false;
    m_download_file_ongoing = false;
    m_operation_remote_path = "";
    m_operation_local_path = "";

    /* Clearing dir contents buffer if previous operation was listing dir contents (and buffer was filled) */
    if (!m_dir_contents_buffer.isEmpty()) {
        qDebug() << "SFTP Client: Clearing dir contents buffer for previous operation...";
        m_dir_contents_received = false;
        qDeleteAll(m_dir_contents_buffer);
        m_dir_contents_buffer.clear();
    }
}

void NetworkConnectorQSsh::sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse)
{
    QString entity = (_is_dir_upload) ? "dir" : "file";
    qDebug() << tr("NetworkConnectorQSsh: Uploading %1 %2 to %3 ...")
                .arg(entity)
                .arg(_local_path)
                .arg(_remote_path);

    /* Init progress indicators */
    quint32 file_count;
    quint64 transfer_size;

    if (_is_dir_upload) {
        transfer_size = FileUtils::dirSize(_local_path, _recurse);
        file_count = FileUtils::fileCount(_local_path, _recurse); // just for logging
    } else {
        QFileInfo file(_local_path); // Assume file exists (checked by calling action)
        transfer_size = file.size();
        file_count = 1;
    }

    qDebug() << QString("NetworkConnectorQSsh: uploading %1 files for a total of %2 bytes...").arg(file_count).arg(transfer_size);

    reinitProgressIndicators(transfer_size);

    QSsh::SftpJobId job;

    if (_is_dir_upload) {
        QString local_dir_path = _local_path;

        if (!_recurse) {
            /* Check if dir has subdirs */
            QDir local_dir(_local_path);
            QStringList subdirs = local_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            if (!subdirs.isEmpty()) {
                qDebug() << QString("SFTP upload dir: source dir '%1' contains subdirectories").arg(_local_path);

                /* Copying root-level files to temp dir */
                QString temp_root_dir_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation) +
                        QDir::separator() + "sftp" + QDir::separator() + "tmpuploaddata";
                QDir temp_root_dir(temp_root_dir_path);
                if (temp_root_dir.exists()) {
                    qDebug() << QString("NetworkConnectorQSsh: cleaning temp dir '%1'").arg(temp_root_dir_path);
                    bool removed = temp_root_dir.removeRecursively();
                    if (!removed) {
                        qWarning() << QString("NetworkConnectorQSsh: could not create temp dir '%1'").arg(temp_root_dir_path);
                    }
                }
                QString temp_dir_path = temp_root_dir_path + QDir::separator() + local_dir.dirName();
                qDebug() << QString("NetworkConnectorQSsh: creating temp dir '%1'").arg(temp_dir_path);
                QDir temp_dir(temp_dir_path);
                bool created = temp_dir.mkpath(".");
                if (!created) {
                    qWarning() << QString("NetworkConnectorQSsh: could not create temp dir '%1'").arg(temp_dir_path);
                }

                qDebug() << QString("Copying source to local temp dir '%1'...").arg(temp_dir_path);

                bool dir_copied = FileUtils::copyDir(_local_path, temp_dir_path, false, true);

                if (!dir_copied) {
                    qCritical() << QString("NetworkConnectorQSsh: copying source to local temp dir '%1' failed, upload aborted");
                    return;
                }

                local_dir_path = temp_dir_path;
            }
        }

        qDebug() << tr("NetworkConnectorQSsh: Uploading dir %1 to %2 ...")
                    .arg(local_dir_path)
                    .arg(_remote_path);

        job = m_channel->uploadDir(local_dir_path, _remote_path);
    } else {
        job = m_channel->uploadFile(_local_path, _remote_path,
                                    QSsh::SftpOverwriteExisting);
    }

    if (job != QSsh::SftpInvalidJob) {
        qDebug() << "NetworkConnectorQSsh upload: Starting job #" << job;
    } else {
        qCritical() << "NetworkConnectorQSsh upload: Invalid Job";
    }
}

void NetworkConnectorQSsh::sl_download(QString _remote_path, QString _local_path, bool _is_dir_download)
{
    if (_is_dir_download) {
        m_download_dir_ongoing = true;
        m_operation_remote_path = _remote_path;
        m_operation_local_path = _local_path;

        /* Start by listing source dir contents to enable progress tracking */
        /* Subdirs recursion not supported (will hang at 100% for a while if dir has subdirs) */
        sl_dirContent(_remote_path, eFileTypeFilter::Files, QStringList(), true);

    } else { // single file download
        qDebug() << tr("NetworkConnectorQSsh: Downloading file %1 to %2 ...")
                    .arg(_remote_path).arg(_local_path);

        m_download_file_ongoing = true;
        reinitProgressIndicators(0); /* Transfer size is not known yet */

        QSsh::SftpJobId job = m_channel->downloadFile(_remote_path, _local_path,
                                                      QSsh::SftpOverwriteExisting);

        if (job != QSsh::SftpInvalidJob) {
            qDebug() << "NetworkConnectorQSsh: Starting job #" << job;
        } else {
            qCritical() << "NetworkConnectorQSsh: Invalid Job";
        }
    }
}

void NetworkConnectorQSsh::sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters, bool _is_for_dir_transfer)
{
    qDebug() << tr("NetworkConnectorQSsh: Listing content from dir %1 ...").arg(_remote_dir_path);

    m_file_type_flags = _flags;
    m_file_filters = _file_filters;

    QSsh::SftpJobId job;

    if (!_is_for_dir_transfer) {
        // Do not signal progress if called prior to dir downloading
        emit si_progressUpdate(10);
        m_last_signalled_progress = 10;
    }

    job = m_channel->listDirectory(_remote_dir_path);

    if (job != QSsh::SftpInvalidJob) {
        qDebug() << "NetworkConnectorQSsh: Starting job #" << job;
    } else {
        qCritical() << "NetworkConnectorQSsh: Invalid Job";
    }
}

void NetworkConnectorQSsh::sl_createRemoteShell(QString& _command)
{
    qDebug() << "NetworkConnectorQSsh: Creating remote shell...";

    m_shell = m_connection->createRemoteShell();

    if (!m_shell) {
        qCritical() << "NetworkConnectorQSsh: Unexpected error null shell";
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

void NetworkConnectorQSsh::startDownloadDir(QString _remote_path, QString _local_path)
{
    qDebug() << tr("NetworkConnectorQSsh: Downloading dir %1 to %2 ...")
                .arg(_remote_path)
                .arg(_local_path);

    QSsh::SftpJobId job;

    job = m_channel->downloadDir(_remote_path, _local_path,
                                 QSsh::SftpOverwriteMode::SftpOverwriteExisting);

    if (job != QSsh::SftpInvalidJob) {
        qDebug() << "NetworkConnectorQSsh: Starting job #" << job;
    } else {
        qCritical() << "NetworkConnectorQSsh: Invalid Job";
    }
}

void NetworkConnectorQSsh::sl_onConnected() {
    qDebug() << "NetworkConnectorQSsh: Connected";

    m_connected = true;
    m_waiting_for_connection = false;

    emit si_connected();
}

void NetworkConnectorQSsh::sl_onDisconnected() {
    qDebug() << "NetworkConnectorQSsh: disconnected";

    QObject* emitter = sender();
    QSsh::SshConnection* expired_connection = static_cast<QSsh::SshConnection*>(emitter);

    /* Case : the connection was closed by calling agent */
    if (m_obsolete_connections.contains(expired_connection)) {
        qDebug() << "NetworkConnectorQSsh: clearing obsolete connection";

        disconnect(expired_connection, SIGNAL(connected()));
        disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
        disconnect(expired_connection, SIGNAL(disconnected()));

        m_obsolete_connections.remove(expired_connection);
        delete expired_connection;

        return;
    }

    /* Unconsistent case : the connection is neither the current connection, nor tracked as an ancient connection */
    if (expired_connection != m_connection) {
        qWarning() << "NetworkConnectorQSsh: unknown connection object, clearing anyway";

        disconnect(expired_connection, SIGNAL(connected()));
        disconnect(expired_connection, SIGNAL(error(QSsh::SshError)));
        disconnect(expired_connection, SIGNAL(disconnected()));

        delete expired_connection;

        return;
    }

    /* Nominal case : the connection was closed after completing job */
    emit si_clearConnection();
}

void NetworkConnectorQSsh::sl_onConnectionError(QSsh::SshError _err) {
    qCritical() << "NetworkConnectorQSsh: Connection error" << _err;

    mapConnectionError(_err);

    QString error_string = m_connection->errorString();
    qCritical() << QString("NetworkConnectorQSsh: SSH error : %1").arg(error_string);

    m_waiting_for_connection = false;

    /* In case of authentication error, prompt for new login
  and give a chance to resume actions. Otherwise clear all */
    if (m_current_cx_error != eConnectionError::AUTHENTICATION_ERROR) {
        emit si_clearConnection();
    }

    emit si_connectionFailed(m_current_cx_error);
}

void NetworkConnectorQSsh::sl_onChannelInitialized() {
    qDebug() << "NetworkConnectorQSsh: Channel Initialized";

    emit si_channelReady();
//    m_current_action->execute();
}

void NetworkConnectorQSsh::sl_onChannelError(const QString& _err) {
    qCritical() << "NetworkConnectorQSsh: Error: " << _err;
}

void NetworkConnectorQSsh::sl_onChannelClosed() {
    qDebug() << "NetworkConnectorQSsh: Channel closed";
    disconnect(this, SLOT(sl_onChannelInitialized()));
    disconnect(this, SLOT(sl_onChannelError(const QString&)));
    disconnect(this, SLOT(sl_onOpfinished(QSsh::SftpJobId, const SftpError,
                                          const QString&)));
    disconnect(this, SLOT(sl_onChannelClosed()));
    disconnect(this, SLOT(sl_onTransferProgress(QSsh::SftpJobId, quint64, quint64)));
    disconnect(this, SLOT(sl_onFileInfoAvailable(QSsh::SftpJobId, const QList<QSsh::SftpFileInfo>)));

    m_channel = NULL;

    emit si_channelClosed();
}

void NetworkConnectorQSsh::sl_onOpfinished(QSsh::SftpJobId _job, const SftpError _error_type,
                                 const QString& _error) {

    if (_error_type != QSsh::SftpError::NoError) {
        qCritical()
                << QString("NetworkConnectorQSsh: Job #%1 failed : %2").arg(_job).arg(_error);

        mapTransferError(_error_type);
        emit si_transferFailed(m_current_tx_error);

        qDebug() << "NetworkConnectorQSsh: Closing channel...";
        m_channel->closeChannel();
        return;
    }

    qDebug() << "NetworkConnectorQSsh: Finished job #" << _job << ": OK";

    if (m_dir_contents_received) { // Case job for listing dir contents complete

        if (m_download_dir_ongoing) {
            // Sub-case : job was started internally prior to downloading

            m_dir_contents_received = false; // uncheck to avoid looping on dir download

            /* Init progress indicators */
            quint64 transfer_size = 0;
            quint32 file_count = m_dir_contents_buffer.size();

            for (NetworkFileInfo *sfi : m_dir_contents_buffer) {
                transfer_size += sfi->size();
            }

            qDebug() << QString("NetworkConnectorQSsh: downloading %1 files for a total of %2 bytes").arg(file_count).arg(transfer_size);

            reinitProgressIndicators(transfer_size);

            startDownloadDir(m_operation_remote_path, m_operation_local_path);
            return; // do not close channel yet
        }

        else { // nominal sub-case : ListDirContents action was explicitely called

            // notify manager
            qDebug() << "NetworkConnectorQSsh: signalling dir contents...";

            /* Copy buffer (may be empty if elements were filtered) */
            QList<NetworkFileInfo*> dir_contents(m_dir_contents_buffer);
            emit si_dirContents(dir_contents);
            /* Channel will be closed hereafter */
        }

    } else { // Case : upload or download job complete

        // notify manager
        qDebug() << "NetworkConnectorQSsh: signalling download or upload complete...";
        emit si_transferFinished();
    }

    qDebug() << "NetworkConnectorQSsh: Closing channel...";
    m_channel->closeChannel();
}

void NetworkConnectorQSsh::sl_onTransferProgress(QSsh::SftpJobId _job, quint64 _progress,
                                       quint64 _total) {

    /* Too verbose logs : activate only for debugging */
    //    qDebug() << QString("NetworkConnectorQSsh: transfer job %1 progress %2 out of %3 bytes")
    //                    .arg(_job)
    //                    .arg(_progress)
    //                    .arg(_total);

    if (m_current_transfer_size == 0) {
        if (m_download_file_ongoing) {
            m_current_transfer_size = _total; // total transfer size is being discovered with current file size
        } else {
            qCritical() << "NetworkConnectorQSsh: current transfer size unknown (0), cannot signal progress";
            return;
        }
    }


    if (m_last_signalled_progress == 100) {
        // already complete ==> skip
        return;
    }

    /* Compute progress increment in bytes */
    quint64 prev_progress = 0;
    if (m_progress_matrix.contains(_job)) {
        prev_progress = m_progress_matrix.value(_job);
    }
    //    qDebug() << QString("NetworkConnectorQSsh: prev progress: %1").arg(prev_progress);
    quint64 increment = _progress - prev_progress;
    m_total_received_bytes += increment;

    /* Update progress matrix */
    m_progress_matrix.insert(_job, _progress); // insert or overwrite existing value

    if (m_total_received_bytes > m_current_transfer_size) {
        qCritical() << QString(
                           "NetworkConnectorQSsh: something went wrong while computing "
                           "progress : received=%1 ; total=%2")
                       .arg(m_total_received_bytes)
                       .arg(m_current_transfer_size);
        return;
    }

    /* Notify client process */
    float progress_rate = (float)m_total_received_bytes / (float)m_current_transfer_size;
    float progress_percentage = progress_rate * 100.0f;
    int rounded_progress = (int)progress_percentage; // round to the lower bound to reduce hanging at 100%
    //    qDebug() << QString("NetworkConnectorQSsh: raw progress indicators:");
    //    qDebug() << QString("received bytes: %1 - transfer size: %2").arg(m_total_received_bytes).arg(m_current_transfer_size);
    //    qDebug() << QString("progress rate: %1 - percentage: %2 - rounded: %3").arg(progress_rate).arg(progress_percentage).arg(rounded_progress);

    if (rounded_progress < m_last_signalled_progress) {
        qCritical() << QString(
                           "NetworkConnectorQSsh: something went wrong while computing "
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

void NetworkConnectorQSsh::sl_onFileInfoAvailable(
        QSsh::SftpJobId _job, const QList<QSsh::SftpFileInfo>& _file_info_list)
{
    Q_UNUSED(_job)

    qDebug() << QString("NetworkConnectorQSsh: Received %1 file info elements")
                .arg(_file_info_list.count());

    m_dir_contents_received = true;

    bool keep_dirs = m_file_type_flags & eFileTypeFilter::Dirs;
    bool keep_files = m_file_type_flags & eFileTypeFilter::Files;

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

    if (!m_download_dir_ongoing) {
        // Do not signal progress if job started internally prior to dir downloading

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


void NetworkConnectorQSsh::sl_closeRemoteShell()
{
    if (!m_shell)
    {
        qCritical() << "NetworkConnectorQSsh: trying to close shell, but shell is null";
        return;
    }

    /* Disconnect all signals but shell closing so that no further output/error is received */
    disconnect(this, SLOT(sl_onShellStarted()));
    disconnect(this, SLOT(sl_onReadyReadStandardOutput()));
    disconnect(this, SLOT(sl_onReadyReadStandardError()));

    m_shell->close();
}

void NetworkConnectorQSsh::sl_executeShellCommand()
{
    QString command_and_nl = m_shell_command.append("\n");

    qDebug()
            << QString("NetworkConnectorQSsh: remote shell send command %1").arg(command_and_nl);

    m_shell->write(command_and_nl.toLatin1());
}

void NetworkConnectorQSsh::sl_onShellStarted() {
    qDebug() << "NetworkConnectorQSsh: Shell started";

    /* Signal 30% progress on shell established */
    emit si_progressUpdate(30);
    m_last_signalled_progress = 30;

    emit si_shellStarted();
    //  m_current_action->execute();
}

void NetworkConnectorQSsh::sl_onReadyReadStandardOutput() {
    qDebug() << "NetworkConnectorQSsh: ready read standard output";

    emit si_readyReadStandardOutput();

    //  if (!m_current_action || m_current_action->isTerminated())
    //  {
    //    qDebug() << "Command action already terminated, ignoring output...";
    //    return;
    //  }
}

QByteArray NetworkConnectorQSsh::readShellStandardOutput() {

    if (!m_shell->isRunning())
    {
        qCritical() << "Shell not running properly, ignoring output...";
        return QByteArray(); // return empty byte array
    }

    QByteArray output_stream = m_shell->readAllStandardOutput();
    //  emit si_shellOutputReceived(output_stream);

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

    return output_stream;
}

void NetworkConnectorQSsh::sl_onReadyReadStandardError() {
    qDebug() << "NetworkConnectorQSsh: ready read standard error";

    emit si_readyReadStandardError();
}

QByteArray NetworkConnectorQSsh::readShellStandardError() {
    if (!m_shell->isRunning())
    {
        qCritical() << "Shell not running properly, ignoring error...";
        return QByteArray(); // return empty byte array
    }

    QByteArray error_stream = m_shell->readAllStandardError();
    //  emit si_shellErrorReceived(error_stream);
    return error_stream;
}


void NetworkConnectorQSsh::sl_onShellClosed(int _exit_status) {
    qDebug() << QString("NetworkConnectorQSsh: Shell closed with exit status %1").arg(_exit_status);

    disconnect(this, SLOT(sl_onShellClosed(int)));

    m_shell = NULL;

    emit si_shellClosed();
}


void NetworkConnectorQSsh::resetConnection() {
    if (m_connected) {
        qDebug() << "NetworkConnectorQSsh: Closing SSH/SFTP connection...";
        m_connection->disconnectFromHost();
        m_connected = false;
        m_waiting_for_connection = false;
        m_obsolete_connections.insert(m_connection); /* keep track of connection object until disconnected */
        m_connection = NULL;

        emit si_clearConnection();

    } else {
        qDebug() << "NetworkConnectorQSsh: SSH/SFTP gateway not connected, ready to reconnect";
    }
}


void NetworkConnectorQSsh::connectToRemoteHost() {
    if (m_waiting_for_connection) {
        qWarning() << QString("NetworkConnectorQSsh: already waiting for connection...");
        return;
    }

    qDebug() << "NetworkConnectorQSsh: connect 1";

    qDebug() << QString("NetworkConnectorQSsh: Connecting to host %1 as %2 ...")
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

void NetworkConnectorQSsh::disconnectFromHost() {
    if (!m_connection) {
        qWarning() << "NetworkConnectorQSsh: already disconnected";
        return;
    }

    qDebug() << QString("NetworkConnectorQSsh: disconnecting from host %1...").arg(m_host);
    m_connection->disconnectFromHost();
}

void NetworkConnectorQSsh::disableConnection() {
    m_connected = false;
    m_waiting_for_connection = false;

    disconnect(this, SLOT(sl_onConnected()));
    disconnect(this, SLOT(sl_onConnectionError(QSsh::SshError)));
    disconnect(this, SLOT(sl_onDisconnected()));
}

void NetworkConnectorQSsh::freeConnection() {
    /* free connection except if it was tracked as obsolete (connection already reset by calling agent) */
    if (m_connection) {
        delete m_connection;
        m_connection = NULL;
    }
}

void NetworkConnectorQSsh::mapConnectionError(QSsh::SshError _err) {
    switch (_err) {
    case QSsh::SshError::SshNoError:
        m_current_cx_error = eConnectionError::NO_ERROR_NC;
        break;

    case QSsh::SshError::SshSocketError:
        m_current_cx_error = eConnectionError::SOCKET_ERROR_NC;
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

    qDebug() << QString("NetworkConnectorQSsh: SSH connection error occurred : ") << m_current_cx_error;
}

void NetworkConnectorQSsh::mapTransferError(QSsh::SftpError _err)
{
    switch (_err) {
    case QSsh::SftpError::NoError:
        m_current_tx_error = eTransferError::NO_ERROR_NC;
        break;

    case QSsh::SftpError::EndOfFile:
        m_current_tx_error = eTransferError::END_OF_FILE;
        break;

    case QSsh::SftpError::FileNotFound:
        m_current_tx_error = eTransferError::FILE_NOT_FOUND;
        break;

    case QSsh::SftpError::PermissionDenied:
        m_current_tx_error = eTransferError::PERMISSION_DENIED;
        break;

    case QSsh::SftpError::GenericFailure:
        m_current_tx_error = eTransferError::GENERIC_FAILURE;
        break;

    case QSsh::SftpError::BadMessage:
        m_current_tx_error = eTransferError::BAD_MESSAGE;
        break;

    case QSsh::SftpError::NoConnection:
        m_current_tx_error = eTransferError::NO_CONNECTION;
        break;

    case QSsh::SftpError::ConnectionLost:
        m_current_tx_error = eTransferError::CONNECTION_LOST;
        break;

    case QSsh::SftpError::UnsupportedOperation:
        m_current_tx_error = eTransferError::UNSUPPORTED_OPERATION;
        break;
    }

    qWarning() << "NetworkConnectorQSsh: SFTP error occurred : " << m_current_tx_error;
}

} // namespace network_tools
