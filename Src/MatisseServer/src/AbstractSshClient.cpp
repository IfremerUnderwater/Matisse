#include "AbstractSshClient.h"

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

AbstractSshClient::AbstractSshClient(QString host, SshClientCredentials *creds, QObject *parent) :
    QObject(parent),
    _host(host)
{
    _creds = creds;

}
