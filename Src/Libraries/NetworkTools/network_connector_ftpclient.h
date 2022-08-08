#ifndef NETWORK_TOOLS_NETWORK_CONNECTOR_FTPCLIENT_H_
#define NETWORK_TOOLS_NETWORK_CONNECTOR_FTPCLIENT_H_

#include "network_connector.h"

#include <QObject>
#include <QtDebug>
#include <QMap>
#include <QFile>
#include <QNetworkSession>
#include <QNetworkConfigurationManager>
#include <QQueue>
#include <QTimer>
#include <QMetaEnum>

#include "network_commons.h"
#include "qftpclient.h"


namespace network_tools {

//Q_NAMESPACE

class NetworkConnectorFTPClient : public NetworkConnector
{
    Q_OBJECT


public:
    explicit NetworkConnectorFTPClient();

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
    void sl_progressUpdate(int _progress);
    void sl_transferFinished();

    void sl_createRemoteShell(QString& _command);
    void sl_closeRemoteShell();
    void sl_executeShellCommand();

    void sl_qftpConnected();

private slots:
    void sl_receiveDirContents(QList<network_tools::NetworkFileInfo*> _dir_contents);

private:

    QFTPClient *m_ftp;

    static const int CONNECTION_TIMEOUT_MS;
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CONNECTOR_FTPCLIENT_H_
