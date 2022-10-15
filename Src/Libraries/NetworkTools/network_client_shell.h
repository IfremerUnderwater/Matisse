#ifndef NETWORK_TOOLS_NETWORK_CLIENT_SHELL_H_
#define NETWORK_TOOLS_NETWORK_CLIENT_SHELL_H_

#include "network_client.h"

namespace network_tools {

class NetworkClientShell : public NetworkClient
{
    Q_OBJECT
public:
    NetworkClientShell();

protected:
    void doInit();
    void doInitBeforeAction();
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

signals:
    void si_commandOutputReceived(NetworkAction *_action, QByteArray _output);
    void si_commandErrorReceived(NetworkAction *_action, QByteArray _error);

protected slots:
    void sl_onShellStarted();
    void sl_onReadyReadStandardOutput();
    void sl_onReadyReadStandardError();
    void sl_onShellClosed();

};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CLIENT_SHELL_H_
