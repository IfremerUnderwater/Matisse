#include "SshClient.h"

using namespace MatisseServer;

SshClientCredentials::SshClientCredentials(QString username, QString password)
{
    if (username.isNull() || username.isEmpty()) {
        qCritical() << "SSH username null or empty";
    }

    _username = username;

    if (password.isNull() || password.isEmpty()) {
        qCritical() << "SSH password null or empty";
    }

    _password = password;
}

SshActionManager::SshActionManager() {

}

SshClient::SshClient(QString host, SshClientCredentials *creds, QObject *parent) :
    SshActionManager(),
    _host(host),
    m_actionQueue(),
    _currentAction(NULL)
{
    _creds = creds;

}

void MatisseServer::SshClient::setConnectionListener(QObject* listener)
{
    _connectionListener = listener;
}

void MatisseServer::SshClient::addAction(SshAction *action)
{
    if (!action->isValid()) {
        qCritical() << "Action type " << action->type() << " cannot be performed, will be skipped";
        return;
    }

    m_actionQueue.enqueue(action);
    if (!_currentAction) {
        processAction();
    }
}

void SshClient::setHost(QString host)
{
    _host = host;
}

void SshClient::setCredentials(SshClientCredentials* creds)
{
    _creds = creds;
}
