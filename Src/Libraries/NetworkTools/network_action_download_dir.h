#ifndef MATISSE_NETWORK_ACTION_DOWNLOAD_DIR_H_
#define MATISSE_NETWORK_ACTION_DOWNLOAD_DIR_H_

#include <QtDebug>
#include "network_file_action.h"

namespace MatisseCommon {

class NetworkActionDownloadDir : public NetworkFileAction
{
    Q_OBJECT
public:
    NetworkActionDownloadDir();

    explicit NetworkActionDownloadDir(QString _remote_dir, QString _local_base_dir);
    void init();
    void execute();
    QString remoteDir() { return m_remote_dir; }
    QString localBaseDir() { return m_local_base_dir; }
    QString progressMessage();

    protected:
        void doTerminate();

    private:
        QString m_remote_dir;
        QString m_local_base_dir;
};

}// namespace MatisseCommon

#endif // MATISSE_NETWORK_ACTION_DOWNLOAD_DIR_H_
