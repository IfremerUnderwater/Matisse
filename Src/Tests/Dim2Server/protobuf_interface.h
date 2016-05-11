#ifndef PROTOBUFINTERFACE_H
#define PROTOBUFINTERFACE_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QList>

#include "proto/nav_photo_info.pb.h"


class QTcpServer;
class QTcpSocket;
class QTimer;


class ProtobufInterface : public QObject
{
    Q_OBJECT
public:
    ProtobufInterface(unsigned int port);
    virtual ~ProtobufInterface();

    bool IsConnected() const;

signals:
    void si_Connected();
    void si_Disconnected();

public slots:
    void sl_OnReceiveNavPhotoInfoMessage(NavPhotoInfoMessage msg);

protected:


private slots:
    // Connection status
    void sl_OnDisconnect();
    void sl_OnDataReceived();
    void sl_OnNewConnection();

private:

    bool InitServer();

    QTcpServer* m_tcp_server;
    QList<QTcpSocket*> m_tcp_socket_list;

    unsigned int m_port;
    bool m_is_connected;

};

#endif // PROTOBUFINTERFACE_H
