#include "basic_connection_wrapper.h"

#include <QSettings>
#include <QUrl>
#include <QDir>

#include "file_utils.h"

using namespace system_tools;

namespace network_tools {

BasicConnectionWrapper::BasicConnectionWrapper() :
    ConnectionWrapper(),
    m_ftp(NULL),
    m_network_session(NULL),
    m_config_manager(),
    m_files_by_job(),
    m_subdirs_buffer(),
    m_dirs_to_upload(),
    m_current_remote_path()
{
}

void BasicConnectionWrapper::resetConnection() {
    qDebug() << QString("BasicConnectionWrapper: reset connection...");
}

void BasicConnectionWrapper::disableConnection() {
    qDebug() << QString("BasicConnectionWrapper: disable connection...");

    m_connected = false;
    m_waiting_for_connection = false;

    disconnect(this, SLOT(sl_onNetworkSessionOpened()));
    disconnect(this, SLOT(sl_onNetworkSessionFailed(QNetworkSession::SessionError)));

    disconnect(this, SLOT(sl_onOpFinished(int,bool)));
    disconnect(this, SLOT(sl_onFileInfoAvailable(QUrlInfo)));
    disconnect(this, SLOT(sl_onTransferProgressReceived(qint64,qint64)));
    disconnect(this, SLOT(sl_onManagerStateChanged(int)));
    disconnect(this, SLOT(sl_onManagerSequenceDone(bool)));
}

void BasicConnectionWrapper::freeConnection() {
    qDebug() << QString("BasicConnectionWrapper: free connection...");

    if (m_ftp) {
        m_ftp->deleteLater();
        m_ftp = NULL;
    }

    if (m_network_session) {
        m_network_session->stop();
        m_network_session->close();
        m_network_session->deleteLater();
        m_network_session = NULL;
    }
}

void BasicConnectionWrapper::reinitBeforeFileOperation() {
    m_download_dir_ongoing = false;
    m_download_file_ongoing = false;
    m_upload_dir_ongoing = false;
    m_recursive_upload = false;
    m_operation_remote_path = "";
    m_operation_local_path = "";
    m_current_remote_path = "";
    m_last_ftp_command = QFtp::Command::None;
    m_upload_local_root_index = 0;
    m_subdirs_buffer.clear();
    m_dirs_to_upload.clear();

    /* Clearing dir contents buffer if previous operation was listing dir contents (and buffer was filled) */
    if (!m_dir_contents_buffer.isEmpty()) {
        qDebug() << "BasicConnectionWrapper: clearing dir contents buffer for previous operation...";
        m_dir_contents_received = false;
        qDeleteAll(m_dir_contents_buffer);
        m_dir_contents_buffer.clear();
    }

    /* Clearing files tracking list */
    if (!m_files_by_job.isEmpty()) {
        qDebug() << "BasicConnectionWrapper: clearing allocated files tracking list for previous operation...";
        qDeleteAll(m_files_by_job);
        m_files_by_job.clear();
    }
}

void BasicConnectionWrapper::connectToRemoteHost() {

    qDebug() << "BsaicConnectionWrapper: 0";

    if (!m_network_session || !m_network_session->isOpen()) {
        qDebug() << "BsaicConnectionWrapper: 1";
        if (m_config_manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
            if (!m_network_session) {
                qDebug() << "BasicConnectionWrapper: retrieving saved network configuration...";
                // Get saved network configuration
                QSettings settings(QSettings::UserScope, QLatin1String("Trolltech"));
                settings.beginGroup(QLatin1String("QtNetwork"));
                const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
                settings.endGroup();

                // If the saved network configuration is not currently discovered use the system default
                QNetworkConfiguration config = m_config_manager.configurationFromIdentifier(id);
                if ((config.state() & QNetworkConfiguration::Discovered) !=
                        QNetworkConfiguration::Discovered) {
                    qWarning() << "BasicConnectionWrapper: saved network configuration not discovered, using system default network configuration";
                    config = m_config_manager.defaultConfiguration();
                }

                m_network_session = new QNetworkSession(config, this);
                connect(m_network_session, SIGNAL(opened()), this, SLOT(sl_onNetworkSessionOpened()));
                connect(m_network_session, SIGNAL(error(QNetworkSession::SessionError)), this, SLOT(sl_onNetworkSessionFailed(QNetworkSession::SessionError)));
            }

            qDebug() << "BasicConnectionWrapper: opening network session...";
            m_network_session->open();
            return;
        }
    }
    qDebug() << "BasicConnectionWrapper: xx";
    sl_onNetworkSessionOpened();

}

void BasicConnectionWrapper::disconnectFromHost() {
    qDebug() << QString("BasicConnectionWrapper: disconnecting from host...");

    if (m_ftp) {
        m_ftp->abort();
    }

    emit si_clearConnection();
}

void BasicConnectionWrapper::sl_onNetworkSessionOpened() {
    m_ftp = new QFtp(this);
    connect(m_ftp, SIGNAL(commandFinished(int,bool)), this, SLOT(sl_onOpFinished(int,bool)));
    connect(m_ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(sl_onFileInfoAvailable(QUrlInfo)));
    connect(m_ftp, SIGNAL(dataTransferProgress(qint64,qint64)), this, SLOT(sl_onTransferProgressReceived(qint64,qint64)));
    connect(m_ftp, SIGNAL(stateChanged(int)), this, SLOT(sl_onManagerStateChanged(int)));
    connect(m_ftp, SIGNAL(done(bool)), this, SLOT(sl_onManagerSequenceDone(bool)));

    qDebug() << QString("BasicConnectionWrapper: Connecting to FTP server %1 as %2...")
                .arg(m_host).arg(m_creds->username());
    m_ftp->connectToHost(m_host, 21);
    m_ftp->login(m_creds->username(), m_creds->password());

    m_waiting_for_connection = true;
}

void BasicConnectionWrapper::sl_onNetworkSessionFailed(QNetworkSession::SessionError _error) {
    qCritical() << "BasicConnectionWrapper: FTP connection error " << _error;
}

void BasicConnectionWrapper::sl_onManagerStateChanged(int _state) {
    QFtp::State new_state = static_cast<QFtp::State>(_state);

    if (m_current_manager_state == _state) {
        qWarning() << QString("BasicConnectionWrapper: FTP manager already in state %1")
                      .arg(m_current_manager_state);
        return;
    }

    qDebug() << QString("BasicConnectionWrapper: FTP manager state changed from %1 to %2")
                .arg(m_current_manager_state).arg(new_state);
    m_current_manager_state = new_state;

    if (m_current_manager_state == QFtp::LoggedIn) {
        m_connected = true;
        m_waiting_for_connection = false;
        m_current_cx_error = eConnectionError::NO_ERROR;
        qDebug() << QString("BasicConnectionWrapper: Connected to FTP server %1 as %2")
                    .arg(m_host).arg(m_creds->username());
        emit si_connected();
    }

    /* How to detect timeout ? */
    if (m_current_manager_state == QFtp::State::Unconnected) {
        qDebug("BasicConnectionWrapper: new state unconnected");
        if (m_ftp) {
            QFtp::Error err = m_ftp->error();
            if (err != QFtp::Error::NoError) {
                qCritical() << "BasicConnectionWrapper: connection lost with error: " << err;
            } else {
                qDebug() << "BasicConnectionWrapper: back to unconnected state";
            }
        }
    }
}

void BasicConnectionWrapper::sl_onManagerSequenceDone(bool _error) {
    if (m_current_cx_error != eConnectionError::NO_ERROR) {
        qDebug() << "BasicConnectionWrapper: FTP sequence aborted with connection error";
        return;
    }

    if (_error) {
        QFtp::Error manager_error = m_ftp->error();
        qCritical() << "BasicConnectionWrapper: an error occurred while executing FTP operations " << manager_error;
        mapTransferError(manager_error);
        emit si_transferFailed(m_current_tx_error);
        disconnectFromHost();
        return;
    }

    qDebug() << QString("BasicConnectionWrapper: FTP sequence complete");
//    QFtp::Command ftp_command = m_ftp->currentCommand();
    qDebug() << QString("BasicConnectionWrapper: last command : %1").arg(m_last_ftp_command);

    bool clear_channel_and_connection = false;

    switch(m_last_ftp_command) {
    case QFtp::Command::Put:
        if (!m_subdirs_buffer.isEmpty()) {
            /* Dir upload: current dir has subdirs */
            qDebug() << QString("BasicConnectionWrapper: recursive upload, checking if subdirs exist on remote server...");
            sl_dirContent(m_current_remote_path, eFileTypeFilter::Dirs, QStringList(), true);
            return;
        } else if (!m_dirs_to_upload.isEmpty()) {
            /* Dir upload: current dir has no subdir but more dir are waiting to be uploaded */
            qDebug() << QString("BasicConnectionWrapper: resuming recursive upload sequence...");
            resumeUploadDir();
            return;
        } else {
            qDebug() << QString("BasicConnectionWrapper: signalling transfer complete");
            emit si_transferFinished();
            clear_channel_and_connection = true;
            break;
        }



    case QFtp::Command::Get:
        qDebug() << QString("BasicConnectionWrapper: signalling transfer complete");
        emit si_transferFinished();
        clear_channel_and_connection = true;
        break;

    case QFtp::Command::List:
        clear_channel_and_connection = true;
        break;

    default:
        break;
    }

    if (clear_channel_and_connection) {
        qDebug() << QString("BasicConnectionWrapper: clearing channel and connection after Put/Get/List sequence...");
        emit si_channelClosed();
//        emit si_clearConnection();
    }
}

void BasicConnectionWrapper::sl_onOpFinished(int _job_id, bool _has_error)  {
    QFtp::Command ftp_command = m_ftp->currentCommand();
    m_last_ftp_command = ftp_command;

    if (_has_error) {
        qCritical() << QString("BasicConnectionWrapper: FTP operation %1 #%2 failed").arg(ftp_command).arg(_job_id);
        QFtp::Error error = m_ftp->error();

        if (ftp_command == QFtp::Command::ConnectToHost || ftp_command == QFtp::Command::Login) {
            mapConnectionError(error);
            m_waiting_for_connection = false;

            /* In case of authentication error, prompt for new login
            and give a chance to resume actions. Otherwise clear all */
            if (m_current_cx_error == eConnectionError::AUTHENTICATION_ERROR) {
                m_ftp->clearPendingCommands();
            } else {
                qDebug() << QString("BasicConnectionWrapper: clearing connection after error occurred...");
                emit si_clearConnection();
            }

            emit si_connectionFailed(m_current_cx_error);

        } else {
            mapTransferError(error);
            emit si_transferFailed(m_current_tx_error);
        }

        return;
    }

    qDebug() << QString("BasicConnectionWrapper: FTP operation %1 #%2 complete").arg(ftp_command).arg(_job_id);

    switch (ftp_command) {

    case QFtp::Command::ConnectToHost:
    case QFtp::Command::Login:
    case QFtp::Command::SetTransferMode:
    case QFtp::Command::Cd:
    case QFtp::Command::Mkdir:
        // nothing more to do
        return;

    case QFtp::Command::Put:
    case QFtp::Command::Get:

        /* Release file allocated for FTP transfer */
        if (m_files_by_job.contains(_job_id)) {
            QFile *file = m_files_by_job.value(_job_id);
//            qDebug() << QString("BasicConnectionWrapper: closing file %1 opened for FTP put command #%2...").arg(file->fileName()).arg(_job_id);
            file->close();
            delete file;
            m_files_by_job.remove(_job_id);
        }

        return;

    default : /* List */
        break;
    }

    if (ftp_command == QFtp::Command::List) {

        if (m_upload_dir_ongoing) {
            // Sub-case : job was started internally prior to uploading
            m_dir_contents_received = false;

            qDebug() << QString("BasicConnectionWrapper: checking existing subdirs...");

            QList<QString> non_existing_subdirs(m_subdirs_buffer);

            for (NetworkFileInfo* nfi : m_dir_contents_buffer) {
                if (!nfi->isDir()) {
                    continue;
                }

                for (QString local_subdir_name : non_existing_subdirs) {
                    if (nfi->name() == local_subdir_name) {
                        qDebug() << QString("BasicConnectionWrapper: remote dir '%1' already exists").arg(local_subdir_name);
                        non_existing_subdirs.removeOne(local_subdir_name);
                        break;
                    }
                }

                if (non_existing_subdirs.isEmpty()) {
                    break;
                }
            }

            for (QString non_existing_subdir_name : non_existing_subdirs) {
                qDebug() << QString("BasicConnectionWrapper: creating remote dir '%1'").arg(non_existing_subdir_name);
                m_ftp->mkdir(non_existing_subdir_name);
            }

            resumeUploadDir();

        } else if (m_download_dir_ongoing && m_dir_contents_received) {
            // Sub-case : job was started internally prior to downloading

            m_dir_contents_received = false; // uncheck to avoid looping on dir download

            /* Init progress indicators */
            quint64 transfer_size = 0;
            quint32 file_count = m_dir_contents_buffer.size();

            for (NetworkFileInfo *sfi : m_dir_contents_buffer) {
                transfer_size += sfi->size();
            }

            qDebug() << QString("BasicConnectionWrapper: downloading %1 files for a total of %2 bytes").arg(file_count).arg(transfer_size);

            reinitProgressIndicators(transfer_size);

            resumeDownloadDir();

            return; // do not close channel yet
        }

        else { // nominal sub-case : ListDirContents action was explicitely called

            // notify manager
            qDebug() << "BasicConnectionWrapper: signalling dir contents...";

            /* Copy buffer (may be empty if elements were filtered) */
            QList<NetworkFileInfo*> dir_contents(m_dir_contents_buffer);
            emit si_dirContents(dir_contents);
        }

    }

//    else if (ftp_command == QFtp::Command::Put) {
////        qDebug() << QString("BasicConnectionWrapper: releasing resources after FTP put command #%1...").arg(_job_id);

//        /* Release file allocated for FTP transfer */
//        if (m_files_by_job.contains(_job_id)) {
//            QFile *file = m_files_by_job.value(_job_id);
////            qDebug() << QString("BasicConnectionWrapper: closing file %1 opened for FTP put command #%2...").arg(file->fileName()).arg(_job_id);
//            file->close();
//            delete file;
//            m_files_by_job.remove(_job_id);
//        }
//    }
}

void BasicConnectionWrapper::sl_onFileInfoAvailable(QUrlInfo _info) {
    QString entry_type = _info.isDir() ? "dir" : "file";
    qDebug() << QString("BasicConnectionWrapper: received %1 info '%2'").arg(entry_type).arg(_info.name());

    m_dir_contents_received = true;

    bool keep_dirs = m_file_type_flags & eFileTypeFilter::Dirs;
    bool keep_files = m_file_type_flags & eFileTypeFilter::Files;

    if (!_info.isValid()) {
        qWarning("BasicConnectionWrapper: received invalid file info, ignoring...");
        return;
    }

    if (_info.isSymLink()) {
        qDebug("BasicConnectionWrapper: received symbolic link file info, ignoring...");
        return;
    }

    QString name = _info.name();
    bool is_dir = _info.isDir();


    /* Skipping dirs if flag not present */
    if (is_dir && !keep_dirs) {
        return;
    }

    /* Skipping files if flag not present */
    if (!is_dir && !keep_files) {
        return;
    }

    /* Skipping Dot and DotDot */
    if (is_dir) {
        if ((name == ".") || (name == "..")) {
            return;
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
            return;
        }
    }

    /* Mapping last modification timestamp from UNIX date/time format */
    QDateTime last_modified = _info.lastModified();

    quint64 size = _info.size();
//    if (!info.sizeValid) {
//        size = 0;
//    }

    NetworkFileInfo* ssh_fi = new NetworkFileInfo(name, is_dir, size, last_modified);
    m_dir_contents_buffer.append(ssh_fi);

    if (!m_download_dir_ongoing && !m_upload_dir_ongoing) {
        // Do not signal progress if job started internally prior to dir downloading

        /* Increment from 50% */
        int new_progress = 0;

        if (m_last_signalled_progress < 50) {
            new_progress = 50;
        } else {
            if (m_last_signalled_progress < 90) {
                new_progress = m_last_signalled_progress + 10;
            }
        }

//        qDebug() << QString("BasicConnectionWrapper: last signalled progress: %1").arg(m_last_signalled_progress);
//        qDebug() << QString("BasicConnectionWrapper: new progress: %1").arg(new_progress);

        if (new_progress > 0) {
            emit si_progressUpdate(new_progress);
            m_last_signalled_progress = new_progress;
        }
    }

}

void BasicConnectionWrapper::sl_onTransferProgressReceived(qint64 _bytes_transferred, qint64 _bytes_total) {
    int job_id = m_ftp->currentId();
//    qDebug() << QString("BasicConnectionWrapper: %1/%2 bytes transferred for job #%3")
//                .arg(_bytes_transferred).arg(_bytes_total).arg(job_id);



    if (m_current_transfer_size == 0) {
        if (m_download_file_ongoing) {
            m_current_transfer_size = _bytes_total; // total transfer size is being discovered with current file size
        } else {
            qCritical() << "BasicConnectionWrapper: current transfer size unknown (0), cannot signal progress";
            return;
        }
    }


    if (m_last_signalled_progress == 100) {
        // already complete ==> skip
        return;
    }

    /* Compute progress increment in bytes */
    quint64 prev_progress = 0;
    if (m_progress_matrix.contains(job_id)) {
        prev_progress = m_progress_matrix.value(job_id);
    }
    //    qDebug() << QString("SecureConnectionWrapper: prev progress: %1").arg(prev_progress);
    quint64 increment = _bytes_transferred - prev_progress;
    m_total_received_bytes += increment;

    /* Update progress matrix */
    m_progress_matrix.insert(job_id, _bytes_transferred); // insert or overwrite existing value

    if (m_total_received_bytes > m_current_transfer_size) {
        qCritical() << QString(
                           "BasicConnectionWrapper: something went wrong while computing "
                           "progress : received=%1 ; total=%2")
                       .arg(m_total_received_bytes)
                       .arg(m_current_transfer_size);
        return;
    }

    /* Notify client process */
    float progress_rate = (float)m_total_received_bytes / (float)m_current_transfer_size;
    float progress_percentage = progress_rate * 100.0f;
    int rounded_progress = (int)progress_percentage; // round to the lower bound to reduce hanging at 100%
//        qDebug() << QString("BasicConnectionWrapper: raw progress indicators:");
//        qDebug() << QString("received bytes: %1 - transfer size: %2").arg(m_total_received_bytes).arg(m_current_transfer_size);
//        qDebug() << QString("progress rate: %1 - percentage: %2 - rounded: %3").arg(progress_rate).arg(progress_percentage).arg(rounded_progress);
//        qDebug() << QString("last signalled progress: %1").arg(m_last_signalled_progress);

    if (rounded_progress < m_last_signalled_progress) {
        qCritical() << QString(
                           "BasicConnectionWrapper: something went wrong while computing "
                           "progress : previous=%1 ; new=%2")
                       .arg(m_last_signalled_progress)
                       .arg(rounded_progress);
        return;
    }

    if (rounded_progress == m_last_signalled_progress) {
        // no visible progress : do not signal
        return;
    }

//    qDebug() << QString("BasicConnectionWrapper: rounded progress: %1").arg(rounded_progress);
    m_last_signalled_progress = rounded_progress;
    emit si_progressUpdate(rounded_progress);
}

QByteArray BasicConnectionWrapper::readShellStandardOutput() {
    qCritical() << "BasicConnectionWrapper: Telnet protocol not supported";
    return QByteArray();
}

QByteArray BasicConnectionWrapper::readShellStandardError() {
    qCritical() << "BasicConnectionWrapper: Telnet protocol not supported";
    return QByteArray();
}

void BasicConnectionWrapper::sl_initFileChannel() {
//    qDebug() << "BasicConnectionWrapper: Channel Initialized";

    reinitBeforeFileOperation();

    /* This step is skipped with QFtp API */
    emit si_channelReady();
}

void BasicConnectionWrapper::sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse) {
    qDebug() << QString("BasicConnectionWrapper: uploading %1 to %2...").arg(_local_path).arg(_remote_path);

    if (_recurse) {
        qCritical() << "BasicConnectionWrapper: recursive upload...";
        m_recursive_upload = true;
    }

    QFileInfo local_file(_local_path);
    if (!local_file.exists()) {
        qCritical() <<  QString("BasicConnectionWrapper: File/Dir '%1' does not exist, impossible to upload")
                        .arg(_local_path);
        return;
    }

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

    qDebug() << QString("BasicConnectionWrapper: uploading %1 files for a total of %2 bytes...").arg(file_count).arg(transfer_size);
    reinitProgressIndicators(transfer_size);

    qDebug() << QString("BasicConnectionWrapper: going to remote dir %1...").arg(_remote_path);
    m_ftp->cd(_remote_path);


    if (_is_dir_upload) {
        m_upload_dir_ongoing = true;
        m_operation_local_path = _local_path;
        m_operation_remote_path = _remote_path;

        QDir local_dir(_local_path);
        QString local_dir_name = local_dir.dirName();
        m_subdirs_buffer.append(local_dir_name); // to check if dir exists at remote end
        m_dirs_to_upload.enqueue(_local_path); // to initiate recursive upload
        m_upload_local_root_index = m_operation_local_path.size() - local_dir_name.size();
        if (m_upload_local_root_index < 0) {
            qCritical() << QString("BasicConnectionWrapper: upload local root index out of range: %1").arg(m_upload_local_root_index);
            m_upload_local_root_index = 0;
        }
        // Check if target dir already exists before proceeding
        sl_dirContent(_remote_path, eFileTypeFilter::Dirs, QStringList(), true);

    } else {
        uploadFile(_local_path);
    }
}

void BasicConnectionWrapper::resumeUploadDir() {
//    QString local_path = m_operation_local_path;
//    if (m_recursive_upload && !m_subdirs_pipe.isEmpty()) {
//        local_path = m_subdirs_pipe.dequeue();
//    }
    if (m_dirs_to_upload.isEmpty()) {
        qWarning() << QString("BasicConnectionWrapper: file tree upload complete");
        return;
    }

    m_subdirs_buffer.clear();
    QString local_path = m_dirs_to_upload.dequeue();

    /* Build absolute remote path */
    QString relative_path = local_path;
    relative_path.remove(0, m_upload_local_root_index);
    QString sep = (m_operation_remote_path.endsWith("/")) ? "" : "/";
    m_current_remote_path = m_operation_remote_path + sep + relative_path;

    qDebug() << QString("BasicConnectionWrapper: changing remote dir to '%1'...").arg(m_current_remote_path);
    m_ftp->cd(m_current_remote_path);
//    m_ftp->setTransferMode(QFtp::TransferMode::Passive);

    /* Uploading dir files */
    QDir local_dir(local_path);
    QFileInfoList dir_entries = local_dir.entryInfoList(QDir::Filter::Files | QDir::Filter::NoDotAndDotDot);

    bool dir_has_files = !dir_entries.isEmpty();

    if (dir_has_files) {
        for (QFileInfo entry: dir_entries) {
            uploadFile(entry.canonicalFilePath());
        }
    }

    if (m_recursive_upload) {
        QFileInfoList subdirs = local_dir.entryInfoList(QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot);
        for (QFileInfo subdir:subdirs) {
            QString subdir_path = subdir.canonicalFilePath();
            qDebug() << QString("BasicConnectionWrapper: current local dir subdir: %1").arg(subdir_path);
            QDir subdir_dir(subdir_path);
            QString subdir_name = subdir_dir.dirName();
            m_subdirs_buffer.append(subdir_name);
            m_dirs_to_upload.enqueue(subdir_path);
        }

        /* if current dir has no files to upload but other dirs are waiting in the queue
         * proceed directly, otherwise wait for files upload sequence to complete */
        if (!dir_has_files) {
            qDebug() << QString("BasicConnectionWrapper: current dir '%1' has no files to upload, skipping...").arg(local_path);
            if (!m_subdirs_buffer.isEmpty()) { // current dir has subdirs
                qDebug() << QString("BasicConnectionWrapper: recursive upload, checking if subdirs exist on remote server...");
                sl_dirContent(m_current_remote_path, eFileTypeFilter::Dirs, QStringList(), true);
            } else if (!m_dirs_to_upload.isEmpty()) { // current dir has no subdirs but more dirs are waiting to be uploaded
                qDebug() << QString("BasicConnectionWrapper: resuming recursive upload sequence...");
                resumeUploadDir();
            } else {
                qDebug() << QString("BasicConnectionWrapper: file tree upload complete, clearing channel and connection...");
                // send signals
                emit si_transferFinished();
                emit si_channelClosed();
            }
        }
    }
}

void BasicConnectionWrapper::uploadFile(QString _local_file_path) {
    QFile *local_file = new QFile(_local_file_path);
    if (!local_file->exists()) {
        qCritical() << QString("BasicConnectionWrapper: File '%1' does not exist, impossible to upload")
                       .arg(_local_file_path);
        delete local_file;
        return;
    }

    /* Open device for file reading */
    bool opened = local_file->open(QIODevice::ReadOnly);
    if (!opened) {
        qCritical() << QString("BasicConnectionWrapper: Unable to open file '%1', impossible to upload")
                       .arg(_local_file_path);
        delete local_file;
        return;
    }

    qDebug() << QString("BasicConnectionWrapper: scheduling upload for file %1...").arg(_local_file_path);
    QFileInfo fi(_local_file_path);
    QString file_name = fi.fileName(); // warning : QFile.fileName() ==> returns full path leading to unknown QFtp error
//    qDebug() << QString("BasicConnectionWrapper: file name %1...").arg(file_name);
    int job_id = m_ftp->put(local_file, file_name); // Binary by default
    m_files_by_job.insert(job_id, local_file); // track file object to release memory when complete
}

void BasicConnectionWrapper::sl_download(QString _remote_path, QString _local_path, bool _is_dir_download) {
    qDebug() << QString("BasicConnectionWrapper: downloading %1 to %2...").arg(_remote_path).arg(_local_path);

    if (_is_dir_download) {
        m_download_dir_ongoing = true;
        m_operation_remote_path = _remote_path;
        m_operation_local_path = _local_path;

        qDebug() << QString("BasicConnectionWrapper: changing remote dir to '%1'...").arg(_remote_path);
        m_ftp->cd(_remote_path);

        /* Start by listing source dir contents to enable progress tracking */
        /* Subdirs recursion not supported (will hang at 100% for a while if dir has subdirs) */
        sl_dirContent(_remote_path, eFileTypeFilter::Files, QStringList(), true);

    } else { // single file download
        qDebug() << tr("SecureConnectionWrapper: Downloading file %1 to %2 ...")
                    .arg(_remote_path).arg(_local_path);

        m_download_file_ongoing = true;
        reinitProgressIndicators(0); /* Transfer size is not known yet */

        if (_remote_path.endsWith("/")) {
            qCritical() << QString("BasicConnectionWrapper: remote path '%1' does not point to a file, aborting download...").arg(_remote_path);
            m_current_tx_error = eTransferError::FILE_NOT_FOUND;
            emit si_transferFailed(m_current_tx_error);
            disconnectFromHost();
            return;
        }

        /* resolving dir and file name */
        int dir_end = _remote_path.lastIndexOf("/");
        QString remote_dir = _remote_path;
        remote_dir.truncate(dir_end);
        QString remote_file_name = _remote_path;
        remote_file_name.remove(0, dir_end + 1);

        qDebug() << QString("BasicConnectionWrapper: changing remote dir to '%1'...").arg(remote_dir);
        m_ftp->cd(remote_dir);
        downloadFile(remote_file_name, _local_path);
    }
}

void BasicConnectionWrapper::resumeDownloadDir() {
    for (NetworkFileInfo *nfi: m_dir_contents_buffer) {
        QString file_name = nfi->name();
        downloadFile(file_name, m_operation_local_path);
    }
}

void BasicConnectionWrapper::downloadFile(QString _remote_file_name, QString _local_dir_path) {

    QString target_local_file_path = _local_dir_path + QDir::separator() + _remote_file_name;
    QFile *target_local_file = new QFile(target_local_file_path);

    /* Open device for file writing */
    bool opened = target_local_file->open(QIODevice::WriteOnly);
    if (!opened) {
        qCritical() << QString("BasicConnectionWrapper: Unable to open local file '%1' for writing, impossible to download")
                       .arg(target_local_file_path);
        delete target_local_file;
        return;
    }

    qDebug() << QString("BasicConnectionWrapper: scheduling download for file '%1' to '%2'").arg(_remote_file_name, _local_dir_path);
    int job_id = m_ftp->get(_remote_file_name, target_local_file); // Binary by default
    m_files_by_job.insert(job_id, target_local_file); // track file object to release memory when complete
}

void BasicConnectionWrapper::sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters, bool _is_for_dir_transfer) {
    qDebug() << QString("BasicConnectionWrapper: listing contents for dir %1...").arg(_remote_dir_path);

    m_file_type_flags = _flags;
    m_file_filters = _file_filters;

    if (!_is_for_dir_transfer) {
        qDebug() << "BasicConnectionWrapper: signalling initial progress for listing dir contents...";
        // Do not signal progress if called prior to dir downloading or uploading
        emit si_progressUpdate(10);
        m_last_signalled_progress = 10;
    }

    m_ftp->list(_remote_dir_path);
}

void BasicConnectionWrapper::mapConnectionError(QFtp::Error _err) {
    switch (_err) {
    case QFtp::NoError:
        m_current_cx_error = eConnectionError::NO_ERROR;
        break;

    case QFtp::HostNotFound:
        m_current_cx_error = eConnectionError::SOCKET_ERROR;
        break;

    case QFtp::ConnectionRefused:
    case QFtp::UnknownError:
        m_current_cx_error = eConnectionError::AUTHENTICATION_ERROR;
        break;

    default:
        qWarning() << "BasicConnectionWrapper: unexpected connection error : " << _err;
        m_current_cx_error = eConnectionError::INTERNAL_ERROR;
        break;
    }

    qDebug() << QString("BasicConnectionWrapper: connection error occurred : ") << m_current_cx_error;
}

void BasicConnectionWrapper::mapTransferError(QFtp::Error _err)
{
    switch (_err) {
    case QFtp::NoError:
        m_current_tx_error = eTransferError::NO_ERROR;
        break;

    case QFtp::NotConnected:
        m_current_tx_error = eTransferError::CONNECTION_LOST;
        break;

    case QFtp::UnknownError:
        m_current_tx_error = eTransferError::BAD_MESSAGE;
        break;

    default:
        qWarning() << "BasicConnectionWrapper: unexpected transfer error : " << _err;
        m_current_tx_error = eTransferError::UNSUPPORTED_OPERATION;
        break;
    }

    qDebug() << QString("BasicConnectionWrapper: transfer error occurred : ") << m_current_tx_error;
}




void BasicConnectionWrapper::sl_createRemoteShell(QString& _command) {
    Q_UNUSED(_command)
    qCritical() << "BasicConnectionWrapper: Telnet protocol not supported";
}

void BasicConnectionWrapper::sl_closeRemoteShell() {
    qCritical() << "BasicConnectionWrapper: Telnet protocol not supported";
}

void BasicConnectionWrapper::sl_executeShellCommand() {
    qCritical() << "BasicConnectionWrapper: Telnet protocol not supported";
}


} // namespace network_tools
