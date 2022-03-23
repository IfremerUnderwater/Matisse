#ifndef NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_
#define NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_

#include <QObject>
#include <QtDebug>
#include <QMap>
#include <QFile>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QQueue>

#include "connection_wrapper.h"
#include "QtFtp/qftp.h"

namespace network_tools {

class BasicConnectionWrapper : public ConnectionWrapper
{
    Q_OBJECT
public:
    explicit BasicConnectionWrapper();

    void resetConnection();

protected:
    void disableConnection();
    void freeConnection();
    void connectToRemoteHost();
    void disconnectFromHost();
    QByteArray readShellStandardOutput();
    QByteArray readShellStandardError();

protected slots:
    void sl_initFileChannel();
    void sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse);
    void sl_download(QString _remote_path, QString _local_path, bool _is_dir_download);
    void sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters, bool _is_for_dir_transfer=false);

    void sl_createRemoteShell(QString& _command);
    void sl_closeRemoteShell();
    void sl_executeShellCommand();

private slots:
    void sl_onNetworkSessionOpened();
    void sl_onNetworkSessionFailed(QNetworkSession::SessionError _error);
    void sl_onManagerStateChanged(int _state);
    void sl_onManagerSequenceDone(bool _error);
    void sl_onOpFinished(int _job_id, bool _has_error);
    void sl_onFileInfoAvailable(QUrlInfo _info);
    void sl_onTransferProgressReceived(qint64 _bytes_transferred, qint64 _bytes_total);

private:
    void reinitBeforeFileOperation();
    void resumeUploadDir();
    void uploadFile(QString _local_file_path);
    void downloadFile(QString _remote_file_name, QString _local_dir_path);
    void resumeDownloadDir();
    void mapConnectionError(QFtp::Error _err);
    void mapTransferError(QFtp::Error _err);

    QFtp *m_ftp;

    QNetworkSession *m_network_session;
    QNetworkConfigurationManager m_config_manager;

    QFtp::State m_current_manager_state = QFtp::State::Unconnected;
    QMap<int, QFile*> m_files_by_job;
    bool m_upload_dir_ongoing = false;
    QFtp::Command m_last_ftp_command = QFtp::Command::None;
    bool m_recursive_upload = false;
    int m_upload_local_root_index = 0;
    QList<QString> m_subdirs_buffer;
    QQueue<QString> m_dirs_to_upload;
    QString m_current_remote_path;
};

} // namespace network_tools

#endif // NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_
