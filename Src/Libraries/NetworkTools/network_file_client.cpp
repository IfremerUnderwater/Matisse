#include "network_file_client.h"

namespace network_tools {

NetworkFileClient::NetworkFileClient() : NetworkClient()
{

}

void NetworkFileClient::doInit()
{
    connect(m_cx_wrapper, SIGNAL(si_channelReady()), SLOT(sl_onChannelReady()));
    connect(m_cx_wrapper, SIGNAL(si_channelClosed()), SLOT(sl_onChannelClosed()));
    connect(m_cx_wrapper, SIGNAL(si_transferFinished()), SLOT(sl_onTransferFinished()));
    connect(m_cx_wrapper, SIGNAL(si_transferFailed(eTransferError)), SLOT(sl_onTransferFailed(eTransferError)));
}

void NetworkFileClient::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_initFileChannel()), m_cx_wrapper, SLOT(sl_initFileChannel()));
    connect(_action, SIGNAL(si_upload(QString, QString, bool, bool)), m_cx_wrapper, SLOT(sl_upload(QString, QString, bool, bool)));
    connect(_action, SIGNAL(si_download(QString, QString, bool)), m_cx_wrapper, SLOT(sl_download(QString, QString, bool)));
    connect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), m_cx_wrapper, SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

void NetworkFileClient::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_initFileChannel()), m_cx_wrapper, SLOT(sl_initFileChannel()));
    disconnect(_action, SIGNAL(si_upload(QString, QString, bool, bool)), m_cx_wrapper, SLOT(sl_upload(QString, QString, bool, bool)));
    disconnect(_action, SIGNAL(si_download(QString, QString, bool)), m_cx_wrapper, SLOT(sl_download(QString, QString, bool)));
    disconnect(_action, SIGNAL(si_dirContent(QString, FileTypeFilters, QStringList)), m_cx_wrapper, SLOT(sl_dirContent(QString, FileTypeFilters, QStringList)));
}

void NetworkFileClient::doInitBeforeAction() {

}

void NetworkFileClient::sl_onChannelReady() {
    qDebug() << "NetworkFileClient: Channel Initialized";
    m_current_action->execute();
}

void NetworkFileClient::sl_onChannelClosed() {
    checkActionPending();
}

void NetworkFileClient::sl_onTransferFailed(eTransferError _error) {
    qDebug() << "NetworkFileClient: before signalling transfer failed";
    if (!m_current_action) {
        qWarning() << "NetworkFileClient: current action null";
    }
    emit si_transferFailed(m_current_action, _error);
    qDebug() << "NetworkFileClient: after signalling transfer failed";
}

void NetworkFileClient::sl_onTransferFinished() {
    emit si_transferFinished(m_current_action);
}

} // namespace network_tools
