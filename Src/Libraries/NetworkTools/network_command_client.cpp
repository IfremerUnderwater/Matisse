#include "network_command_client.h"

namespace network_tools {

NetworkCommandClient::NetworkCommandClient() :
    NetworkClient()
{

}

void NetworkCommandClient::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_createRemoteShell(QString&)), SLOT(sl_createRemoteShell(QString&)));
    connect(_action, SIGNAL(si_closeRemoteShell()), SLOT(sl_closeRemoteShell()));
    connect(_action, SIGNAL(si_executeCommand()), SLOT(sl_executeCommand()));
}

void NetworkCommandClient::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_createRemoteShell(QString&)), this, SLOT(sl_createRemoteShell(QString&)));
    disconnect(_action, SIGNAL(si_closeRemoteShell()), this, SLOT(sl_closeRemoteShell()));
    disconnect(_action, SIGNAL(si_executeCommand()), this, SLOT(sl_executeCommand()));
}

} // namespace network_tools

