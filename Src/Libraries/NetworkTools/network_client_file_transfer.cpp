#include "network_client_file_transfer.h"

namespace network_tools {

NetworkClientFileTransfer::NetworkClientFileTransfer() : NetworkClient()
{

}

void NetworkClientFileTransfer::doInit()
{
    connect(m_connector, SIGNAL(si_channelReady()), SLOT(sl_onChannelReady()));
    connect(m_connector, SIGNAL(si_channelClosed()), SLOT(sl_onChannelClosed()));
    connect(m_connector, SIGNAL(si_transferFinished()), SLOT(sl_onTransferFinished()));
    connect(m_connector, SIGNAL(si_transferFailed(eTransferError)), this, SLOT(sl_onTransferFailed(eTransferError)));
    connect(m_connector, SIGNAL(si_dirContents(QList<NetworkFileInfo *>)), this, SLOT(sl_onDirContentsReceived(QList<NetworkFileInfo *>)));

    /* internal wiring to decouple the FTP error sequence from the GUI client */
    connect(this, SIGNAL(si_transferFailedInternal(NetworkAction::eNetworkActionType, eTransferError)), this, SLOT(sl_onTransferFailedInternal(NetworkAction::eNetworkActionType, eTransferError)), Qt::ConnectionType::QueuedConnection);
}

void NetworkClientFileTransfer::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_initFileChannel()), m_connector, SLOT(sl_initFileChannel()));
    connect(_action, SIGNAL(si_upload(QString, QString, bool, bool)), m_connector, SLOT(sl_upload(QString, QString, bool, bool)));
    connect(_action, SIGNAL(si_download(QString, QString, bool)), m_connector, SLOT(sl_download(QString, QString, bool)));
    connect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), m_connector, SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

void NetworkClientFileTransfer::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_initFileChannel()), m_connector, SLOT(sl_initFileChannel()));
    disconnect(_action, SIGNAL(si_upload(QString, QString, bool, bool)), m_connector, SLOT(sl_upload(QString, QString, bool, bool)));
    disconnect(_action, SIGNAL(si_download(QString, QString, bool)), m_connector, SLOT(sl_download(QString, QString, bool)));
    disconnect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), m_connector, SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

void NetworkClientFileTransfer::doInitBeforeAction() {

}

void NetworkClientFileTransfer::sl_onChannelReady() {
    qDebug() << "NetworkClientFileTransfer: Channel Initialized";
    m_current_action->execute();
}

void NetworkClientFileTransfer::sl_onChannelClosed() {
    checkActionPending();
}

void NetworkClientFileTransfer::sl_onTransferFailed(eTransferError _error) {
    if (!m_current_action) {
        qWarning() << "NetworkClientFileTransfer: current action null";
    }

    emit si_transferFailedInternal(m_current_action->type(), _error);
}

void NetworkClientFileTransfer::sl_onTransferFailedInternal(NetworkAction::eNetworkActionType _action_type, eTransferError _error) {
//    qDebug() << "NetworkClientFileTransfer: transfer failed internal loop";
    emit si_transferFailed(_action_type, _error);
}

void NetworkClientFileTransfer::sl_onTransferFinished() {
    emit si_transferFinished(m_current_action);
}

void NetworkClientFileTransfer::sl_onDirContentsReceived(QList<NetworkFileInfo *> _contents) {
    emit si_dirContents(m_current_action, _contents);
}

} // namespace network_tools
