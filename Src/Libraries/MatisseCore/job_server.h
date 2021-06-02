#ifndef MATISSE_JOB_SERVER_H_
#define MATISSE_JOB_SERVER_H_

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include "process_data_manager.h"

///
/// Class for serving jobs list and results location to QGIS plugin
///


namespace matisse {

class JobServer : public QObject
{
    Q_OBJECT
public:
    explicit JobServer(int port);

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
};

} // namespace matisse

#endif // MATISSE_JOB_SERVER_H_
