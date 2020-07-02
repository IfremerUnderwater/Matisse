#ifndef SSHCLIENTSTUB_H
#define SSHCLIENTSTUB_H

#include "SshClient.h"

namespace MatisseCommon {

class SshClientStub : public SshClient
{
    Q_OBJECT

public:
    explicit SshClientStub(QObject *parent = nullptr);

    void connectToHost();
    void disconnectFromHost();

    void resume();

protected:
    void createSftpChannel();
    void createRemoteShell(QString& command);
    void createRemoteProcess(QString& command);
    void executeCommand();
    void upload(QString localPath, QString remotePath, bool isDirUpload); // récupérer flux
    void uploadDir(QString localDir, QString remoteBaseDir);
//    void download(QString remoteFilePath, QString localPath); // récupérer flux
//    QStringList listDirs(QString parentDirPath);
//    QStringList listFiles(QString parentDirPath);

    void init();
    void processAction();
};

}

#endif // SSHCLIENTSTUB_H
