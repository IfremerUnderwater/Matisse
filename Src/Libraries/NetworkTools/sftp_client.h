#ifndef NETWORK_TOOLS_SFTP_CLIENT_H_
#define NETWORK_TOOLS_SFTP_CLIENT_H_

#include "sshconnection.h"
#include "sftpchannel.h"
#include "network_action.h"
#include "network_file_client.h"

using namespace QSsh;

namespace network_tools {

class SftpClient :
    public NetworkFileClient
{
    Q_OBJECT

public:
    explicit SftpClient();

    void resume();
    void init();
    void resetConnection();
    void clearActions();

signals:

protected slots:
    void sl_initFileChannel();
    void sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload);
    void sl_download(QString _remote_path, QString _local_path, bool _is_dir_download);
    void sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters);

        void sl_onConnected();
        void sl_onDisconnected();
        void sl_onConnectionError(QSsh::SshError _err);
        void sl_onChannelInitialized();
        void sl_onChannelError(const QString& _err);
        void sl_onChannelClosed();
        void sl_onOpfinished(QSsh::SftpJobId _job, const SftpError _error_type, const QString& _error = QString());
        void sl_onTransferProgress(QSsh::SftpJobId _job, quint64 _progress, quint64 _total);
        void sl_onFileInfoAvailable(QSsh::SftpJobId _job, const QList<QSsh::SftpFileInfo>& _file_info_list);

    private:
        void processAction();
        void clearConnectionAndActionQueue();
        void connectToRemoteHost();

        void startDownloadDir(QString _remote_path, QString _local_path);
        void reinitProgressIndicators(quint64 _transfer_size, quint32 _matrix_size);
        void mapConnectionError(QSsh::SshError _err);
        void mapTransferError(QSsh::SftpError _err);

        QString m_shell_command;
        QList<NetworkFileInfo*> m_dir_contents_buffer;
        bool m_dir_contents_received;
        FileTypeFilters m_file_type_flags;
        QStringList m_file_filters;
        quint64 m_current_transfer_size = 0;
        QVector<quint64> m_progress_matrix;
        quint64 m_total_received_bytes = 0;
        quint32 m_progress_offset = 0; // first job id for progress tracking
        
        QSsh::SftpChannel::Ptr m_channel;
        QSsh::SshConnection* m_connection;
        QSet<QSsh::SshConnection*> m_obsolete_connections;
    };

} // namespace network_tools

#endif  // NETWORK_TOOLS_SFTP_CLIENT_H_
