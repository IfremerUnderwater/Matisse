﻿#ifndef JOBSERVER_H
#define JOBSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "Xml.h"
#include "ProcessDataManager.h"

namespace MatisseTools {
class JobServer : public QObject
{
    Q_OBJECT
public:
    explicit JobServer(int port, ProcessDataManager *processDataManager);

    void sendExecutionNotification(QString name);
signals:

private slots:
    void slot_clientConnected();
    void slot_clientDisconnected();
    void slot_readData();
private:
    bool sendCmd(QString data);

private:
    QTcpServer _server;
    QTcpSocket *_socket;
    //Xml *_xml;
    ProcessDataManager *_processDataManager;
};
}

#endif // JOBSERVER_H
