#include "network_client_shell.h"

namespace network_tools {

NetworkClientShell::NetworkClientShell() :
    NetworkClient()
{

}

void NetworkClientShell::doInit()
{
    connect(m_connector, SIGNAL(si_shellStarted()), SLOT(sl_onShellStarted()));
    connect(m_connector, SIGNAL(si_readyReadStandardOutput()), SLOT(sl_onReadyReadStandardOutput()));
    connect(m_connector, SIGNAL(si_readyReadStandardError()), SLOT(sl_onReadyReadStandardError()));
    connect(m_connector, SIGNAL(si_shellClosed()), SLOT(sl_onShellClosed()));
}

void NetworkClientShell::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_createRemoteShell(QString&)), m_connector, SLOT(sl_createRemoteShell(QString&)));
    connect(_action, SIGNAL(si_closeRemoteShell()), m_connector, SLOT(sl_closeRemoteShell()));
    connect(_action, SIGNAL(si_executeShellCommand()), m_connector, SLOT(sl_executeShellCommand()));
}

void NetworkClientShell::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_createRemoteShell(QString&)), m_connector, SLOT(sl_createRemoteShell(QString&)));
    disconnect(_action, SIGNAL(si_closeRemoteShell()), m_connector, SLOT(sl_closeRemoteShell()));
    disconnect(_action, SIGNAL(si_executeShellCommand()), m_connector, SLOT(sl_executeShellCommand()));
}

void NetworkClientShell::doInitBeforeAction() {

}

void NetworkClientShell::sl_onShellStarted() {
    m_current_action->execute();
}

void NetworkClientShell::sl_onReadyReadStandardOutput() {
    qDebug() << "NetworkClientShell: ready read standard output";

    if (!m_current_action || m_current_action->isTerminated())
    {
        qDebug() << "NetworkClientShell: command action already terminated, ignoring output...";
        return;
    }

    QByteArray output = m_connector->readShellStandardOutput();
    if (!output.isEmpty()) {
        emit si_commandOutputReceived(m_current_action, output);
    }
}

void NetworkClientShell::sl_onReadyReadStandardError() {
    qDebug() << "NetworkClientShell: ready read standard error";

    if (!m_current_action || m_current_action->isTerminated()) {
        qWarning() << "NetworkClientShell: command action already terminated, ignoring error...";
        return;
    }

    QByteArray error = m_connector->readShellStandardError();
    if (!error.isEmpty()) {
        emit si_commandErrorReceived(m_current_action, error);
    }
}

void NetworkClientShell::sl_onShellClosed() {
    checkActionPending();
}

} // namespace network_tools

