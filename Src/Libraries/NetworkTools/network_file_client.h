#ifndef MATISSE_NETWORK_FILE_CLIENT_H_
#define MATISSE_NETWORK_FILE_CLIENT_H_

#include "network_client.h"

namespace MatisseCommon {


class NetworkFileClient : public NetworkClient
{
    Q_OBJECT

public:
    NetworkFileClient();

signals:
    void si_transferFinished(NetworkAction *_action);
    void si_transferFailed(NetworkAction *_action, TransferError _err);
    void si_dirContents(QList<NetworkFileInfo *> _contents);

protected slots:
    virtual void sl_initFileChannel() = 0;
    virtual void sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload) = 0;
    virtual void sl_download(QString _remote_path, QString _local_path, bool _is_dir_download) = 0;
    virtual void sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters) = 0;

protected:
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

    TransferError m_current_tx_error = TransferError::NoError;
};

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_FILE_CLIENT_H_
