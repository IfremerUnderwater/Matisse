#include "SshClientStub.h"

using namespace MatisseCommon;

SshClientStub::SshClientStub(QObject *parent) :
    SshClient(parent)
{

}

void SshClientStub::connectToHost()
{
    qDebug() << tr("Connecting to host %1 as %2 ...").arg(m_host).arg(m_creds->username());
}

void SshClientStub::disconnectFromHost()
{
    qDebug() << tr("Disconnecting from host %1 ...").arg(m_host);
}

void SshClientStub::resume()
{
}

void SshClientStub::createSftpChannel()
{
}

void SshClientStub::createRemoteShell(QString& command)
{
}

void SshClientStub::createRemoteProcess(QString& command)
{
}

void SshClientStub::executeCommand()
{
}

void SshClientStub::upload(QString localPath, QString remotePath, bool isDirUpload)
{
    qDebug() << tr("Uploading file %1 to %2 ...").arg(localPath).arg(remotePath);
}

void SshClientStub::uploadDir(QString localDir, QString remoteBaseDir)
{
    qDebug() << tr("Uploading dir %1 to %2 ...").arg(localDir).arg(remoteBaseDir);
}

void SshClientStub::init()
{
    // nothing
}

void SshClientStub::processAction()
{
    qDebug() << QString("Processing new action");
}
