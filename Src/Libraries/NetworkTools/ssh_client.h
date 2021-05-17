#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QtDebug>
#include "sshconnection.h"
#include "sshremoteprocess.h"
#include "network_action.h"
#include "network_command_client.h"

using namespace QSsh;

namespace MatisseCommon {

class SshClient : public NetworkCommandClient
{
    Q_OBJECT
public:
    SshClient();

    void init();

    /* TODO mutualize code in a connection wrapper class */
    void resume();
    void resetConnection();
    void clearActions();

protected slots:
    void sl_createRemoteShell(QString& _command);
    void sl_closeRemoteShell();
    void sl_executeCommand();

    /* TODO mutualize code in a connection wrapper class */
    void sl_onConnected();
    void sl_onDisconnected();
    void sl_onConnectionError(QSsh::SshError);

    void sl_onShellStarted();
    void sl_onReadyReadStandardOutput();
    void sl_onReadyReadStandardError();
    void sl_onShellClosed(int _exit_status);

private:
    /* TODO mutualize code in a connection wrapper class */
    void processAction();
    void clearConnectionAndActionQueue();
    void connectToRemoteHost();
    void mapConnectionError(QSsh::SshError _err);


    SshRemoteProcess::Ptr m_shell;
    SshConnection* m_connection;
    QSet<SshConnection*> m_obsolete_connections;

    QString m_shell_command;
};

} // namespace MatisseCommon

#endif // SSHCLIENT_H
