#ifndef MATISSE_NETWORK_ACTION_SEND_COMMAND_H_
#define MATISSE_NETWORK_ACTION_SEND_COMMAND_H_

#include <QtDebug>
#include "network_command_action.h"

namespace MatisseCommon {

class NetworkActionSendCommand : public NetworkCommandAction
{
    Q_OBJECT
public:
    explicit NetworkActionSendCommand(QString _command_string);
    void init();
    void execute();
    QString command() { return m_command; }
    void setResponse(QString _response) { m_response = _response; }
    QString response() { return m_response; }
    QString progressMessage();

protected:
    void doTerminate();

private:
    QString m_command;
    QString m_response;
};

} // namespace MatisseCommon

#endif // MATISSE_NETWORK_ACTION_SEND_COMMAND_H_
