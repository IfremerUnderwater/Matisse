#ifndef MATISSE_NETWORK_ACTION_UPLOAD_FILE_H_
#define MATISSE_NETWORK_ACTION_UPLOAD_FILE_H_

#include <QtDebug>
#include "network_file_action.h"

namespace MatisseCommon {

class NetworkActionUploadFile : public NetworkFileAction
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

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_ACTION_UPLOAD_FILE_H_
