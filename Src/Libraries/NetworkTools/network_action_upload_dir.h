#ifndef NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_DIR_H_
#define NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_DIR_H_

#include <QtDebug>

#include "network_file_action.h"

namespace network_tools {

class NetworkActionUploadDir : public NetworkFileAction
{
    Q_OBJECT
public:
    explicit NetworkActionUploadDir(QString _local_dir, QString _remote_base_dir);
    void init();
    void execute();
    QString localDir() { return m_local_dir; }
    QString remoteBaseDir() { return m_remote_base_dir; }
    QString progressMessage();

protected:
    void doTerminate();

private:
    QString m_local_dir;
    QString m_remote_base_dir;
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_DIR_H_
