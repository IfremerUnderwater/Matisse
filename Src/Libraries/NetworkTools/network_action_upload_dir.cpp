#include "network_action_upload_dir.h"

namespace network_tools {

NetworkActionUploadDir::NetworkActionUploadDir(QString _local_dir, QString _remote_base_dir, bool _recurse) :
    NetworkActionFileTransfer(eNetworkActionType::UploadDir),
    m_recurse(_recurse)
{
    QFileInfo info(_local_dir);

    if (!info.exists()) {
        qCritical() << QString("NetworkActionUploadDir: %1 cannot be uploaded : dir does not exist")
                       .arg(_local_dir);
        m_is_valid = false;
    }

    m_local_dir = info.canonicalFilePath();
    m_remote_base_dir = _remote_base_dir;
}

void NetworkActionUploadDir::init()
{
    qDebug() << "NetworkActionUploadDir: Init file channel";
    emit si_initFileChannel();
}

void NetworkActionUploadDir::execute()
{
    qDebug() << "NetworkActionUploadDir: Before upload";
    emit si_upload(m_local_dir, m_remote_base_dir, true, m_recurse);
    qDebug() << "NetworkActionUploadDir: After upload";
}

QString NetworkActionUploadDir::progressMessage()
{
    return QString("Uploading files to '%1'")
              .arg(m_remote_base_dir);
}

void NetworkActionUploadDir::doTerminate()
{

}

} // namespace network_tools
