#include "LibSshClient.h"

using namespace MatisseServer;

LibSshClient::LibSshClient(QString host, SshClientCredentials *creds,  QObject *parent) :
    AbstractSshClient(host, creds, parent)
{

}

void LibSshClient::connect()
{
    qDebug() << tr("Connecting to host %1 as %2 ...").arg(_host).arg(_creds->username());
}

void LibSshClient::disconnect()
{
    qDebug() << tr("Disconnecting from host %1 ...").arg(_host);
}

void LibSshClient::upload(QString localFilePath, QString remotePath)
{
    qDebug() << tr("Uploading file %1 to %2 ...").arg(localFilePath).arg(remotePath);
}

