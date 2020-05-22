#ifndef SSHCLIENTSTUB_H
#define SSHCLIENTSTUB_H

#include "AbstractSshClient.h"

namespace MatisseServer {

class SshClientStub : public AbstractSshClient
{
    Q_OBJECT

public:
    explicit SshClientStub(QString host, SshClientCredentials * creds, QObject *parent = nullptr);

    void connect();
    void disconnect();
    void upload(QString localFilePath, QString remotePath); // récupérer flux
//    void sendCommand(QString command, QString & response); // rajouter un paramètre flux d'entrée pour l'information intermédiaire (statut de copie par ex.)
//    void download(QString remoteFilePath, QString localPath); // récupérer flux
//    QStringList listDirs(QString parentDirPath);
//    QStringList listFiles(QString parentDirPath);
};

}

#endif // SSHCLIENTSTUB_H
