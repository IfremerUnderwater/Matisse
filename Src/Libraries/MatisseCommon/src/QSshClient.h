#ifndef MATISSE_QSSH_CLIENT_H_
#define MATISSE_QSSH_CLIENT_H_

#include "SshClient.h"

#include "sshconnection.h"
#include "sftpchannel.h"
#include "sshremoteprocess.h"
#include "SshAction.h"

using namespace QSsh;

namespace MatisseCommon {

    class QSshClient :
        public SshClient
    {
        Q_OBJECT

    public:
        explicit QSshClient(QObject* parent = nullptr);

        void connectToHost();
        void disconnectFromHost();
        void resume();
                
    //    void download(QString remoteFilePath, QString localPath); // récupérer flux
    //    QStringList listDirs(QString parentDirPath);
    //    QStringList listFiles(QString parentDirPath);
        void init();

    protected:
        void upload(QString localPath, QString remotePath, bool isDirUpload); // récupérer flux
        void uploadDir(QString localDir, QString remoteBaseDir);
        void processAction();

    signals:

    protected slots:
        void onConnected();
        void onDisconnected();
        void clearConnectionAndActionQueue();
        void onConnectionError(QSsh::SshError);
        void onChannelInitialized();
        void onChannelError(const QString& err);
        void onChannelClosed();
        void onOpfinished(QSsh::SftpJobId job, const SftpError errorType, const QString& error = QString());
        void onTransferProgress(QSsh::SftpJobId job, quint64 progress, quint64 total);

        void onShellStarted();
        void onReadyReadStandardOutput();
        void onReadyReadStandardError();
        /*
         * Parameter is of type ExitStatus, but we use int because of
         * signal/slot awkwardness (full namespace required).
         */
        void onShellClosed(int exitStatus);

    private:
        void connectToRemoteHost();
        void createSftpChannel();
        void createRemoteShell(QString& command);
        void closeRemoteShell();
        void createRemoteProcess(QString& command);
        void executeCommand();
        void mapError(QSsh::SshError err);

        QString m_local_path;
        QString m_remote_path;
        QString m_shell_command;
        
        SftpChannel::Ptr m_channel;
        SshRemoteProcess::Ptr m_shell;
        SshConnection* m_connection;
        
        bool m_is_dir_upload;
    };

}

#endif  // MATISSE_QSSH_CLIENT_H_