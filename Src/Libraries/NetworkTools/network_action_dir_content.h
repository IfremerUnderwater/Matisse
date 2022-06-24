#ifndef NETWORK_TOOLS_NETWORK_ACTION_DIR_CONTENT_H_
#define NETWORK_TOOLS_NETWORK_ACTION_DIR_CONTENT_H_

#include <QtDebug>
#include "network_commons.h"
#include "network_action_file_transfer.h"

namespace network_tools {

class NetworkActionDirContent : public NetworkActionFileTransfer
{
    Q_OBJECT
public:
    explicit NetworkActionDirContent(
            QString _remote_dir,
            FileTypeFilters _flags = eFileTypeFilter::Dirs,
            QStringList _file_filters = QStringList());
    void init();
    void execute();
    QString remoteDir() { return m_remote_dir; }
    QString progressMessage();

signals:
    void si_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters);

protected:
    void doTerminate();

private:
    QString m_remote_dir;
    FileTypeFilters m_flags;
    QStringList m_file_filters;
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_ACTION_DIR_CONTENT_H_
