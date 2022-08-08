#include "network_connector_ftpclient.h"

#include "file_utils.h"

#include <QSettings>
#include <QUrl>
#include <QDir>


using namespace system_tools;
using namespace embeddedmz;

namespace network_tools {

const int NetworkConnectorFTPClient::CONNECTION_TIMEOUT_MS = 30000;

NetworkConnectorFTPClient::NetworkConnectorFTPClient() :
    m_ftp(NULL)
{
}

void NetworkConnectorFTPClient::resetConnection() {
    qDebug() << QString("NetworkConnectorFTPClient: reset connection...");
}

// To check
void NetworkConnectorFTPClient::disableConnection() {
    qDebug() << QString("NetworkConnectorFTPClient: disable connection...");

    m_connected = false;
    m_waiting_for_connection = false;

}

void NetworkConnectorFTPClient::freeConnection() {
    qDebug() << QString("NetworkConnectorFTPClient: free connection...");

    if (m_ftp) {
        //m_ftp->CleanupSession();
        m_ftp->stopThread(); // without this application would crash deleting a running thread
		delete m_ftp;
        m_ftp = NULL;
    }

}

void NetworkConnectorFTPClient::connectToRemoteHost() {


    if (!m_connected)
	{
        if (!m_ftp)
        {
            m_ftp = new QFTPClient();
            connect(m_ftp, SIGNAL(si_connected()), this, SLOT(sl_qftpConnected()));
            connect(m_ftp, SIGNAL(si_dirContents(QList<network_tools::NetworkFileInfo*>)), this, SLOT(sl_receiveDirContents(QList<network_tools::NetworkFileInfo*>)));
            connect(m_ftp, SIGNAL(si_progressUpdate(int)), this, SLOT(sl_progressUpdate(int)));
            connect(m_ftp, SIGNAL(si_transferFinished()), this, SLOT(sl_transferFinished()));
        }
		
        m_ftp->connectToHost(m_host, m_creds->username(), m_creds->password(), 21);

	}
	
}

void NetworkConnectorFTPClient::disconnectFromHost() {
    qDebug() << QString("NetworkConnectorFTPClient: disconnecting from host...");

    if (m_ftp) {
        //m_ftp->CleanupSession();
    }

    emit si_clearConnection();
}


QByteArray NetworkConnectorFTPClient::readShellStandardOutput() {
    qCritical() << "NetworkConnectorFTPClient: Telnet protocol not supported";
    return QByteArray();
}

QByteArray NetworkConnectorFTPClient::readShellStandardError() {
    qCritical() << "NetworkConnectorFTPClient: Telnet protocol not supported";
    return QByteArray();
}

void NetworkConnectorFTPClient::sl_initFileChannel() {
//    qDebug() << "NetworkConnectorFTPClient: Channel Initialized";

    /* This step is skipped with FTPClient API */
    emit si_channelReady();
}

void NetworkConnectorFTPClient::sl_upload(QString _local_path, QString _remote_path, bool _is_dir_upload, bool _recurse) {
    qDebug() << QString("NetworkConnectorFTPClient: uploading %1 to %2...").arg(_local_path).arg(_remote_path);

    if(_is_dir_upload)
        m_ftp->uploadDir(_local_path, _remote_path, _recurse);
    else
        m_ftp->uploadFile(_local_path, _remote_path);

}

void NetworkConnectorFTPClient::sl_download(QString _remote_path, QString _local_path, bool _is_dir_download) {
    qDebug() << QString("NetworkConnectorFTPClient: downloading %1 to %2...").arg(_remote_path).arg(_local_path);


}



void NetworkConnectorFTPClient::sl_dirContent(QString _remote_dir_path, FileTypeFilters _flags, QStringList _file_filters, bool _is_for_dir_transfer) {
    qDebug() << QString("NetworkConnectorFTPClient: listing contents for dir %1...").arg(_remote_dir_path);
    m_ftp->listDir(_remote_dir_path);

}

void NetworkConnectorFTPClient::sl_receiveDirContents(QList<network_tools::NetworkFileInfo*> _dir_contents)
{
    emit si_dirContents(_dir_contents);

    // don't do anything but needed by manager for other libs
    emit si_channelClosed();
}


void NetworkConnectorFTPClient::sl_createRemoteShell(QString& _command) {
    Q_UNUSED(_command)
    qCritical() << "NetworkConnectorFTPClient: Telnet protocol not supported";
}

void NetworkConnectorFTPClient::sl_closeRemoteShell() {
    qCritical() << "NetworkConnectorFTPClient: Telnet protocol not supported";
}

void NetworkConnectorFTPClient::sl_executeShellCommand() {
    qCritical() << "NetworkConnectorFTPClient: Telnet protocol not supported";
}

void NetworkConnectorFTPClient::sl_qftpConnected()
{
    m_connected = true;
    emit si_connected();
}

void NetworkConnectorFTPClient::sl_progressUpdate(int _progress)
{
    emit si_progressUpdate(_progress);
}

void NetworkConnectorFTPClient::sl_transferFinished()
{
    emit si_transferFinished();
}

} // namespace network_tools
