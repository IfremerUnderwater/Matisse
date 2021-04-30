#ifndef MATISSE_QSSH_CLIENT_H_
#define MATISSE_QSSH_CLIENT_H_

#include "SshClient.h"

#include "sshconnection.h"
#include "sftpchannel.h"
#include "sshremoteprocess.h"
#include "network_action.h"

using namespace QSsh;

namespace MatisseCommon {

class QSshClient :
    public NetworkClient
{
    Q_OBJECT

public:
    explicit QSshClient(QObject* parent = nullptr);

    void resume();
    void init();
    void resetConnection();
    void clearActions();

    protected:
     void upload(QString _local_path, QString _remote_path, bool is_dir_upload);
     void download(QString _remote_path, QString _local_path, bool _is_dir_upload);
     void dirContent(QString _remote_dir_path,
       FileTypeFilters _flags =
       FileTypeFilter::AllEntries,
       QStringList _file_filters = QStringList());

    signals:

    protected slots:
        void sl_onConnected();
        void sl_onDisconnected();
        void sl_onConnectionError(QSsh::SshError);
        void sl_onChannelInitialized();
        void sl_onChannelError(const QString& err);
        void sl_onChannelClosed();
        void sl_onOpfinished(QSsh::SftpJobId job, const SftpError errorType, const QString& error = QString());
        void sl_onTransferProgress(QSsh::SftpJobId job, quint64 progress, quint64 total);

        void sl_onShellStarted();
        void sl_onReadyReadStandardOutput();
        void sl_onReadyReadStandardError();
        /*
         * Parameter is of type ExitStatus, but we use int because of
         * signal/slot awkwardness (full namespace required).
         */
        void sl_onShellClosed(int exitStatus);
        void sl_onFileInfoAvailable(QSsh::SftpJobId job, const QList<QSsh::SftpFileInfo>& fileInfoList); 

    private:
        void processAction();
        void clearConnectionAndActionQueue();
        void connectToRemoteHost();
        void createSftpChannel();
        void createRemoteShell(QString& command);
        void closeRemoteShell();
        void executeCommand();
        void startDownloadDir(QString _remote_path, QString _local_path);
        void reinitProgressIndicators(quint64 _transfer_size, quint32 _matrix_size);
        void mapConnectionError(QSsh::SshError _err);
        void mapTransferError(QSsh::SftpError _err);

        QString m_local_path;
        QString m_remote_path;
        QString m_shell_command;
        QList<NetworkFileInfo*> m_dir_contents_buffer;
        bool m_dir_contents_received;
        FileTypeFilters m_file_type_flags;
        QStringList m_file_filters;
        quint64 m_current_transfer_size = 0;
        QVector<quint64> m_progress_matrix;
        quint64 m_total_received_bytes = 0;
        int m_last_signalled_progress = 0;
        quint32 m_progress_offset = 0; // first job id for progress tracking
        
        SftpChannel::Ptr m_channel;
        SshRemoteProcess::Ptr m_shell;
        SshConnection* m_connection;
        QSet<SshConnection*> m_obsolete_connections;
    };

}

#endif  // MATISSE_QSSH_CLIENT_H_