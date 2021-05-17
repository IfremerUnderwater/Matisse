#ifndef MATISSE_NETWORK_ACTION_DIR_CONTENT_H_
#define MATISSE_NETWORK_ACTION_DIR_CONTENT_H_

#include <QtDebug>
#include "network_file_action.h"

namespace MatisseCommon {

class NetworkActionDirContent : public NetworkFileAction
{
    Q_OBJECT
public:
    explicit NetworkActionDirContent(
        QString _remote_dir,
        FileTypeFilters _flags = FileTypeFilter::Dirs,
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

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_ACTION_DIR_CONTENT_H_
