#ifndef MATISSE_NETWORK_COMMAND_ACTION_H_
#define MATISSE_NETWORK_COMMAND_ACTION_H_

#include "network_action.h"

namespace MatisseCommon {

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

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_COMMAND_ACTION_H_
