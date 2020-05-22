#ifndef REMOTEJOBMANAGER_H
#define REMOTEJOBMANAGER_H

#include <QObject>
#include <QtDebug>
#include "AbstractSshClient.h"


namespace MatisseServer {

class RemoteJobManager : public QObject
{
    Q_OBJECT
public:
    explicit RemoteJobManager(QObject *parent = nullptr);

    void uploadDataset();
    void uploadJobFiles(QString remoteJobPath);
    void scheduleJob();
    void downloadDataset();

    void setSshClient(AbstractSshClient *sshClient);

signals:


private slots:


private:
    AbstractSshClient *_sshClient;

private:



};

}

#endif // REMOTEJOBMANAGER_H
