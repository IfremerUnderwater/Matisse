#ifndef NETWORK_TOOLS_COMMAND_CLIENT_H_
#define NETWORK_TOOLS_COMMAND_CLIENT_H_

#include "network_client.h"

namespace network_tools {

class NetworkCommandClient : public NetworkClient
{
    Q_OBJECT
public:
    NetworkCommandClient();

signals:
    void si_shellOutputReceived(NetworkAction *_action, QByteArray _output);
    void si_shellErrorReceived(NetworkAction *_action, QByteArray _error);

protected slots:
    virtual void sl_createRemoteShell(QString& _command) = 0;
    virtual void sl_closeRemoteShell() = 0;
    virtual void sl_executeCommand() = 0;

protected:
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

};

} // namespace network_tools

#endif // NETWORK_TOOLS_COMMAND_CLIENT_H_
