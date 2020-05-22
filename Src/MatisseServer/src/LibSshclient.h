#ifndef LIBSSHCLIENT_H
#define LIBSSHCLIENT_H

#include <QObject>
#include "AbstractSshClient.h"

//namespace MatisseServer {


class LibSshClient : public MatisseServer::AbstractSshClient
{
    Q_OBJECT
public:
    explicit LibSshClient(QString host, MatisseServer::SshClientCredentials *creds, QObject *parent = nullptr);

    void connect();
    void disconnect();
    void upload(QString localFilePath, QString remotePath); // récupérer flux
//    void sendCommand(QString command, QString & response); // rajouter un paramètre flux d'entrée pour l'information intermédiaire (statut de copie par ex.)
//    void download(QString remoteFilePath, QString localPath); // récupérer flux
//    QStringList listDirs(QString parentDirPath);
//    QStringList listFiles(QString parentDirPath);

signals:

};

//}

#endif // LIBSSHCLIENT_H
