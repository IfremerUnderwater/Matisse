#ifndef MATISSE_NETWORK_FILE_ACTION_H_
#define MATISSE_NETWORK_FILE_ACTION_H_

#include "network_action.h"

namespace MatisseCommon {

class NetworkFileAction : public NetworkAction
{
    Q_OBJECT
public:
    NetworkFileAction(NetworkActionType _type);

signals:
    void si_initFileChannel();
    void si_upload(QString _local_path, QString _remote_path, bool is_dir_upload);
    void si_download(QString _remote_path, QString _local_path, bool _is_dir_upload);
    void si_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters);
};

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_FILE_ACTION_H_
