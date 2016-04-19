#ifndef NAVPHOTOINFOTCPLISTENER_H
#define NAVPHOTOINFOTCPLISTENER_H

#include <QObject>
#include <QtNetwork/QTcpSocket>
#include <QTimer>
#include "proto/nav_photo_info.pb.h"

class NavPhotoInfoTcpListener : public QObject
{
    Q_OBJECT
public:
    explicit NavPhotoInfoTcpListener();
    virtual ~NavPhotoInfoTcpListener();

    bool isConnected() const;

signals:
    void signal_NavPhotoInfoMessage(NavPhotoInfoMessage msg);


public slots:
    void slot_Connect(QString hostname_p, int port_p);
    void slot_disconnect();

    void slot_OnDataReceived();

    void slot_OnConnectionTimer();
    void slot_OnConnect();
    void slot_OnDisconnect();
private:
    QTimer *_connectionTimer;
    QTcpSocket * _tcpSocket;
    bool _isConnected;
    bool _connectionAllowed;
    QString _hostname;
    int _hostport;

    void Connect();
};

#endif // NAVPHOTOINFOTCPLISTENER_H
