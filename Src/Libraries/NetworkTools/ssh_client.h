#ifndef NETWORK_TOOLS_SSH_CLIENT_H_
#define NETWORK_TOOLS_SSH_CLIENT_H_

#include <QtDebug>
#include "sshconnection.h"
#include "sshremoteprocess.h"
#include "network_action.h"
#include "network_command_client.h"

namespace network_tools {

class SshClient : public NetworkCommandClient
{
    Q_OBJECT
public:
    SshClient();


    /* TODO mutualize code in a connection wrapper class */
//    void resume();
//    void resetConnection();
//    void clearActions();

protected:
//    void doInit();
//    void doInitBeforeAction();
//    void processAction();
//    void connectToRemoteHost();

protected slots:
//    void sl_createRemoteShell(QString& _command);
//    void sl_closeRemoteShell();
//    void sl_executeCommand();

//    void sl_onConnected();
//    void sl_onDisconnected();
//    void sl_onConnectionError(QSsh::SshError _err);

//    void sl_onShellStarted();
//    void sl_onReadyReadStandardOutput();
//    void sl_onReadyReadStandardError();
//    void sl_onShellClosed();

//    void sl_onShellOutputReceived(QByteArray _output);
//    void sl_onShellErrorReceived(QByteArray _error);

private:
    /* TODO mutualize code in a connection wrapper class */
//    void clearConnectionAndActionQueue();
//    void mapConnectionError(QSsh::SshError _err);


//    QSsh::SshRemoteProcess::Ptr m_shell;
//    QSsh::SshConnection* m_connection;
//    QSet<QSsh::SshConnection*> m_obsolete_connections;

//    QString m_shell_command;
};

} // namespace network_tools

#endif // NETWORK_TOOLS_SSH_CLIENT_H_
