#include "RemoteJobManager.h"
#include <QFileInfo>

using namespace MatisseServer;

RemoteJobManager::RemoteJobManager(QObject *parent) :
    QObject(parent)
{
}

void RemoteJobManager::uploadJobFiles(QString jobBundle)
{
    qDebug() << "Uploading job files...";

    _sshClient->connect();
    _sshClient->upload(jobBundle, "/home/matisse");
    _sshClient->disconnect();
}

void RemoteJobManager::setSshClient(AbstractSshClient *sshClient)
{
    if (!sshClient) {
        qFatal("SSH client implementation is null");
    }

    _sshClient = sshClient;
}






