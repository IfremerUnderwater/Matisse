#ifndef NETWORK_TOOLS_NETWORK_FILE_CLIENT_H_
#define NETWORK_TOOLS_NETWORK_FILE_CLIENT_H_

#include "network_client.h"

namespace network_tools {

class NetworkFileClient : public NetworkClient
{
    Q_OBJECT

public:
    NetworkFileClient();

signals:
    void si_transferFinished(NetworkAction *_action);
    void si_transferFailed(NetworkAction *_action, eTransferError _err);
    void si_dirContents(QList<NetworkFileInfo *> _contents);

protected slots:
    virtual void sl_initFileChannel() = 0;
    virtual void sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse) = 0;
    virtual void sl_download(QString _remote_path, QString _local_path, bool _is_dir_download) = 0;
    virtual void sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters) = 0;

protected:
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

    eTransferError m_current_tx_error = eTransferError::NO_ERROR;
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_FILE_CLIENT_H_
