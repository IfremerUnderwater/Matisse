#include "network_action_download_dir.h"

namespace network_tools {

NetworkActionDownloadDir::NetworkActionDownloadDir(QString _remote_dir,
                                                   QString _local_base_dir) :
    NetworkActionFileTransfer(eNetworkActionType::DownloadDir)
{
    QFileInfo info(_local_base_dir);

    if (!info.exists()) {
        qCritical() << QString(
                       "NetworkActionDownloadDir: Remote dir cannot be downloaded to '%1' : target dir does not exist")
                       .arg(_local_base_dir);
        m_is_valid = false;
    }

    m_local_base_dir = info.canonicalFilePath();
    m_remote_dir = _remote_dir;
}

void NetworkActionDownloadDir::init()
{
    emit si_initFileChannel();
}

void NetworkActionDownloadDir::execute()
{
    emit si_download(m_remote_dir, m_local_base_dir, true);
}

QString NetworkActionDownloadDir::progressMessage() {
    return QString("Downloading files from '%1'").arg(m_remote_dir);
}

void NetworkActionDownloadDir::doTerminate()
{

}

} // namespace network_tools
