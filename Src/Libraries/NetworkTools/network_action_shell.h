#ifndef NETWORK_TOOLS_NETWORK_ACTION_SHELL_H_
#define NETWORK_TOOLS_NETWORK_ACTION_SHELL_H_

#include "network_action.h"

namespace network_tools {

class NetworkActionShell : public NetworkAction
{
    Q_OBJECT
public:
    NetworkActionShell(eNetworkActionType _type);

signals:
    void si_createRemoteShell(QString& _command);
    void si_closeRemoteShell();
    void si_executeShellCommand();
};

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_ACTION_SHELL_H_
