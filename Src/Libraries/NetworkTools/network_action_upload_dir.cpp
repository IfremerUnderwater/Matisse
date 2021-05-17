#include "network_action_upload_dir.h"

namespace MatisseCommon {

NetworkActionUploadDir::NetworkActionUploadDir(QString _local_dir, QString _remote_base_dir) :
    NetworkFileAction(NetworkActionType::UploadDir)
{
    QFileInfo info(_local_dir);

    if (!info.exists()) {
        qCritical() << QString("QSshClient: %1 cannot be uploaded : dir does not exist")
                       .arg(_local_dir);
        m_is_valid = false;
    }

    m_local_dir = info.canonicalFilePath();
    m_remote_base_dir = _remote_base_dir;
}

void NetworkActionUploadDir::init()
{
    emit si_initFileChannel();
}

void NetworkActionUploadDir::execute()
{
    emit si_upload(m_local_dir, m_remote_base_dir, true);
}

QString NetworkActionUploadDir::progressMessage()
{
    return QString("Uploading files to '%1'")
              .arg(m_remote_base_dir);
}

void NetworkActionUploadDir::doTerminate()
{

}

} // namespace MatisseCommon
