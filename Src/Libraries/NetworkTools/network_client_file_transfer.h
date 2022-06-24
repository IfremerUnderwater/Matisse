#ifndef NETWORK_TOOLS_NETWORK_CLIENT_FILE_TRANSFER_H_
#define NETWORK_TOOLS_NETWORK_CLIENT_FILE_TRANSFER_H_

#include "network_commons.h"
#include "network_client.h"

namespace network_tools {

class NetworkClientFileTransfer : public NetworkClient
{
    Q_OBJECT

public:
    NetworkClientFileTransfer();

protected:
    void doInit();
    void doInitBeforeAction();
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

signals:
    void si_transferFinished(NetworkAction *_action);
    void si_transferFailed(NetworkAction::eNetworkActionType, eTransferError _err);
    void si_transferFailedInternal(NetworkAction::eNetworkActionType, eTransferError _err);
    void si_dirContents(NetworkAction * _action, QList<NetworkFileInfo *> _contents);

protected slots:

    void sl_onChannelReady();
    void sl_onChannelClosed();
    void sl_onTransferFailed(eTransferError _error);
    void sl_onTransferFinished();
    void sl_onDirContentsReceived(QList<NetworkFileInfo *> _contents);

    /* Internal slot used to desynchronize the error processing sequence from the UI thread */
    void sl_onTransferFailedInternal(NetworkAction::eNetworkActionType, eTransferError _error);
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CLIENT_FILE_TRANSFER_H_
