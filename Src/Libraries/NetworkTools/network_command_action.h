#ifndef NETWORK_TOOLS_COMMAND_ACTION_H_
#define NETWORK_TOOLS_COMMAND_ACTION_H_

#include "network_action.h"

namespace network_tools {

class NetworkCommandAction : public NetworkAction
{
    Q_OBJECT
public:
    NetworkCommandAction(NetworkActionType _type);

signals:
    void si_createRemoteShell(QString& _command);
    void si_closeRemoteShell();
    void si_executeCommand();
};

} // namespace network_tools

#endif // NETWORK_TOOLS_COMMAND_ACTION_H_
