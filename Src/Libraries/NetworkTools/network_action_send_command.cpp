#include "network_action_send_command.h"

namespace network_tools {

NetworkActionSendCommand::NetworkActionSendCommand(QString _command_string) :
    NetworkCommandAction(eNetworkActionType::SendCommand),
    m_command(_command_string)
{

}

void NetworkActionSendCommand::init() {
    qDebug() << "NetworkActionSendCommand: init";
    emit si_createRemoteShell(m_command);
}

void NetworkActionSendCommand::execute() {
    qDebug() << "NetworkActionSendCommand: execute";
    emit si_executeShellCommand();
}

void NetworkActionSendCommand::doTerminate() {
    qDebug() << "NetworkActionSendCommand: closing remote shell";
    emit si_closeRemoteShell();
}

QString NetworkActionSendCommand::progressMessage() {
    return QString("Executing command '%1'").arg(m_command); }

} // namespace network_tools
