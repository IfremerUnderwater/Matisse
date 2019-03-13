#ifndef JOBSERVER_H
#define JOBSERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "ProcessDataManager.h"

///
/// Class for serving jobs list and results location to QGIS plugin
///



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
    ProcessDataManager *_processDataManager;
    QTcpSocket *_socket;
    //Xml *_xml;
};
}

#endif // JOBSERVER_H
