#ifndef SSHCLIENT_H
#define SSHCLIENT_H

#include <QObject>
#include <QtDebug>

namespace MatisseServer {

class SshClientCredentials
{
public:
    explicit SshClientCredentials(QString _username, QString _password);

    QString username() { return _username; };
    QString password() { return _password; };

private:
    QString _username;
    QString _password;
};

class AbstractSshClient : public QObject
{
    Q_OBJECT

public:
    explicit AbstractSshClient(QString host, SshClientCredentials *creds, QObject *parent = nullptr);
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    virtual void upload(QString localFilePath, QString remotePath) = 0; // récupérer flux
//    virtual void sendCommand(QString command, QString & response); // rajouter un paramètre flux d'entrée pour l'information intermédiaire (statut de copie par ex.)
//    virtual void download(QString remoteFilePath, QString localPath); // récupérer flux
//    virtual QStringList listDirs(QString parentDirPath);
//    virtual QStringList listFiles(QString parentDirPath);

signals:

public slots:
//    void slot_opeComplete(QString response);
//    void slot_opeAborted(quint16 errorCode, QString errorMessage);
//    void slot_opeInfoStream(QString infoStream);

protected:
    QString _host;
    SshClientCredentials *_creds;
};

}

#endif // SSHCLIENT_H
