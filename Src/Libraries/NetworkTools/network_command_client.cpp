#include "network_command_client.h"

namespace network_tools {

NetworkCommandClient::NetworkCommandClient() :
    NetworkClient()
{

}

void NetworkCommandClient::doInit()
{
    connect(m_cx_wrapper, SIGNAL(si_shellStarted()), SLOT(sl_onShellStarted()));
    connect(m_cx_wrapper, SIGNAL(si_readyReadStandardOutput()), SLOT(sl_onReadyReadStandardOutput()));
    connect(m_cx_wrapper, SIGNAL(si_readyReadStandardError()), SLOT(sl_onReadyReadStandardError()));
    connect(m_cx_wrapper, SIGNAL(si_shellClosed()), SLOT(sl_onShellClosed()));
}

void NetworkCommandClient::connectAction(NetworkAction *_action)
{
    connect(_action, SIGNAL(si_createRemoteShell(QString&)), m_cx_wrapper, SLOT(sl_createRemoteShell(QString&)));
    connect(_action, SIGNAL(si_closeRemoteShell()), m_cx_wrapper, SLOT(sl_closeRemoteShell()));
    connect(_action, SIGNAL(si_executeShellCommand()), m_cx_wrapper, SLOT(sl_executeShellCommand()));
}

void NetworkCommandClient::disconnectAction(NetworkAction *_action)
{
    disconnect(_action, SIGNAL(si_createRemoteShell(QString&)), m_cx_wrapper, SLOT(sl_createRemoteShell(QString&)));
    disconnect(_action, SIGNAL(si_closeRemoteShell()), m_cx_wrapper, SLOT(sl_closeRemoteShell()));
    disconnect(_action, SIGNAL(si_executeShellCommand()), m_cx_wrapper, SLOT(sl_executeShellCommand()));
}

void NetworkCommandClient::doInitBeforeAction() {

}

void NetworkCommandClient::sl_onShellStarted() {
    m_current_action->execute();
}

void NetworkCommandClient::sl_onReadyReadStandardOutput() {
    qDebug() << "NetworkCommandClient: ready read standard output";

    if (!m_current_action || m_current_action->isTerminated())
    {
        qDebug() << "NetworkCommandClient: command action already terminated, ignoring output...";
        return;
    }

    QByteArray output = m_cx_wrapper->readShellStandardOutput();
    if (!output.isEmpty()) {
        emit si_commandOutputReceived(m_current_action, output);
    }
}

void NetworkCommandClient::sl_onReadyReadStandardError() {
    qDebug() << "NetworkCommandClient: ready read standard error";

    if (!m_current_action || m_current_action->isTerminated()) {
        qWarning() << "NetworkCommandClient: command action already terminated, ignoring error...";
        return;
    }

    QByteArray error = m_cx_wrapper->readShellStandardError();
    if (!error.isEmpty()) {
        emit si_commandErrorReceived(m_current_action, error);
    }
}

void NetworkCommandClient::sl_onShellClosed() {
    checkActionPending();
}

} // namespace network_tools

