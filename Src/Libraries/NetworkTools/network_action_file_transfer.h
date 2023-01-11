#ifndef NETWORK_TOOLS_NETWORK_ACTION_FILE_TRANSFER_H_
#define NETWORK_TOOLS_NETWORK_ACTION_FILE_TRANSFER_H_

#include "network_action.h"

namespace network_tools {

class NetworkActionFileTransfer : public NetworkAction
{
    Q_OBJECT
public:
    NetworkActionFileTransfer(eNetworkActionType _type);

signals:
    void si_initFileChannel();
    void si_upload(QString _local_path, QString _remote_path, bool is_dir_upload, bool _recurse);
    void si_download(QString _remote_path, QString _local_path, bool _is_dir_upload);
    void si_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters);
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_ACTION_FILE_TRANSFER_H_
