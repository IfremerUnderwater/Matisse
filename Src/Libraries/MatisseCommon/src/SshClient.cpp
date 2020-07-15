#include "SshClient.h"

using namespace MatisseCommon;

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

SshClient::SshClient(QObject *parent) :
    SshActionManager(),
    m_action_queue(),
    m_current_action(NULL)
{
}

void SshClient::addAction(SshAction *action)
{
    if (!action->isValid()) {
        qCritical() << "Action type " << action->type() << " cannot be performed, will be skipped";
        return;
    }

    m_action_queue.enqueue(action);
    if (!m_current_action) {
        processAction();
    }
}

void SshClient::setHost(QString host)
{
    m_host = host;
}

void SshClient::setCredentials(SshClientCredentials* creds)
{
    m_creds = creds;
}
