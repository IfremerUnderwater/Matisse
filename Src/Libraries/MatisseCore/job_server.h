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
    explicit JobServer(int _port);

    void sendExecutionNotification(QString _name);
signals:

private slots:
    void sl_clientConnected();
    void sl_clientDisconnected();
    void sl_readData();
private:
    bool sendCmd(QString _data);

private:
    QTcpServer m_server;
    QTcpSocket *m_socket;
};

} // namespace matisse

#endif // MATISSE_JOB_SERVER_H_
