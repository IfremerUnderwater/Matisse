#ifndef MATISSE_NETWORK_COMMAND_CLIENT_H_
#define MATISSE_NETWORK_COMMAND_CLIENT_H_

#include "network_client.h"

namespace MatisseCommon {

class NetworkCommandClient : public NetworkClient
{
    Q_OBJECT
public:
    NetworkCommandClient();

signals:
    void si_shellOutputReceived(NetworkAction *action, QByteArray output);
    void si_shellErrorReceived(NetworkAction *action, QByteArray error);

protected slots:
    virtual void sl_createRemoteShell(QString& _command) = 0;
    virtual void sl_closeRemoteShell() = 0;
    virtual void sl_executeCommand() = 0;

protected:
    void connectAction(NetworkAction *_action);
    void disconnectAction(NetworkAction *_action);

};

} // MatisseCommon

#endif // MATISSE_NETWORK_COMMAND_CLIENT_H_
