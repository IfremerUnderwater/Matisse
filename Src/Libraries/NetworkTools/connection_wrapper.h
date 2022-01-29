#ifndef NETWORK_CLIENT_CONNECTION_WRAPPER_H_
#define NETWORK_CLIENT_CONNECTION_WRAPPER_H_

#include <QObject>

#include "network_commons.h"
#include "network_credentials.h"
#include "network_file_info.h"

namespace network_tools {

class ConnectionWrapper : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionWrapper();

    bool isConnected();
    bool isWaitingForConnection();
    void setHost(QString _host);
    QString host();
    void setCredentials(NetworkCredentials* _creds);
    QString username();

    virtual void resetConnection() = 0;
    virtual QByteArray readShellStandardOutput() = 0;
    virtual QByteArray readShellStandardError() = 0;

protected:
    virtual void disableConnection() = 0;
    virtual void freeConnection() = 0;
    virtual void connectToRemoteHost() = 0;
    virtual void disconnectFromHost() = 0;

    friend class NetworkClient;
    friend class NetworkCommandClient;
    friend class NetworkFileClient;

signals:
    void si_connected();
    void si_connectionFailed(eConnectionError _err);
    void si_clearConnection();
//    void si_connectionClosed();
    void si_progressUpdate(int _progress);
    void si_channelReady();
    void si_channelClosed();
    void si_transferFailed(eTransferError _error);
    void si_transferFinished();
    void si_dirContents(QList<NetworkFileInfo *> _contents);

    void si_shellStarted();
    void si_shellClosed();
    void si_readyReadStandardOutput();
    void si_readyReadStandardError();
//    void si_shellOutputReceived(QByteArray _output);
//    void si_shellErrorReceived(QByteArray _error);

protected slots:
    virtual void sl_initFileChannel() = 0;
    virtual void sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse) = 0;
    virtual void sl_download(QString _remote_path, QString _local_path, bool _is_dir_download) = 0;
    virtual void sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters, bool _is_for_dir_download=false) = 0;

    virtual void sl_createRemoteShell(QString& _command) = 0;
    virtual void sl_closeRemoteShell() = 0;
    virtual void sl_executeCommand() = 0;

protected:
    QString m_host;
    NetworkCredentials *m_creds;
    bool m_connected = false;
    bool m_waiting_for_connection = false;
    eConnectionError m_current_cx_error = eConnectionError::NO_ERROR;
    eTransferError m_current_tx_error = eTransferError::NO_ERROR;
    int m_last_signalled_progress = 0;
    QString m_shell_command;
};

} // namespace network_tools

#endif // NETWORK_CLIENT_CONNECTION_WRAPPER_H_
