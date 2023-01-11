#ifndef NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_FILE_H_
#define NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_FILE_H_

#include <QtDebug>
#include "network_action_file_transfer.h"

namespace network_tools {

class NetworkActionUploadFile : public NetworkActionFileTransfer
{
    Q_OBJECT
public:
    explicit NetworkActionUploadFile(QString _local_file_path, QString _remote_path);
    void init();
    void execute();
    QString localFilePath() { return m_local_file_path; }
    QString remotePath() { return m_remote_path; }
    QString progressMessage();

protected:
    void doTerminate();

private:
    QString m_local_file_path;
    QString m_remote_path;

};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_ACTION_UPLOAD_FILE_H_
