#include "network_action_upload_file.h"

namespace network_tools {

NetworkActionUploadFile::NetworkActionUploadFile(QString _local_file_path, QString _remote_path) :
    NetworkFileAction(eNetworkActionType::UploadFile)
{
    QFileInfo info(_local_file_path);

    if (!info.exists()) {
        qCritical() << QString("QSshClient: %1 cannot be uploaded : file does not exist")
                       .arg(_local_file_path);
        m_is_valid = false;
    }

    m_local_file_path = info.canonicalFilePath();
    m_remote_path = _remote_path + QLatin1Char('/') + info.fileName();
}

void NetworkActionUploadFile::init()
{
    emit si_initFileChannel();
}

void NetworkActionUploadFile::execute()
{
    emit si_upload(m_local_file_path, m_remote_path, false, false);
}

QString NetworkActionUploadFile::progressMessage()
{
    return QString("Uploading file to '%1'").arg(m_remote_path);
}

void NetworkActionUploadFile::doTerminate()
{

}

} // namespace network_tools
