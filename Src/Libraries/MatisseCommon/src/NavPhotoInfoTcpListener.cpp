#include "NavPhotoInfoTcpListener.h"
#include <QThread>

NavPhotoInfoTcpListener::NavPhotoInfoTcpListener() : QObject()
{
    _connectionTimer = new QTimer(this);
    _tcpSocket = new QTcpSocket(this);

    _isConnected = false;
    _hostname = QString("none");

}

NavPhotoInfoTcpListener::~NavPhotoInfoTcpListener()
{
    if(_tcpSocket)
        delete _tcpSocket;
    if (_connectionTimer)
        delete _connectionTimer;
}

void NavPhotoInfoTcpListener::slot_Connect(QString hostname_p, int port_p)
{
    _hostname = hostname_p;
    _hostport = port_p;

    _connectionTimer->setInterval(2000);
    connect(_connectionTimer, SIGNAL(timeout()), this, SLOT(slot_OnConnectionTimer()));

    connect(_tcpSocket, SIGNAL(disconnected()), this, SLOT(slot_OnDisconnect()));
    connect(_tcpSocket, SIGNAL(connected()), this, SLOT(slot_OnConnect()));
    connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(slot_OnDataReceived()));

    Connect();
}

void NavPhotoInfoTcpListener::Connect()
{
    qDebug() << " NavPhotoInfo Thread = " << QThread::currentThread();
    _tcpSocket->connectToHost(_hostname, _hostport);
    _connectionTimer->start();
}

void NavPhotoInfoTcpListener::slot_disconnect(){
    _tcpSocket->disconnectFromHost();
    _connectionTimer->stop();
}

void NavPhotoInfoTcpListener::slot_OnDataReceived()
{
    QByteArray data_received = _tcpSocket->readAll();

    if (!data_received.isEmpty()){

        NavPhotoInfoMessage msg;
        msg.ParseFromArray(data_received.data(), data_received.size());

        emit signal_NavPhotoInfoMessage(msg);

    }

}
bool NavPhotoInfoTcpListener::isConnected() const
{
    return _isConnected;
}


void NavPhotoInfoTcpListener::slot_OnDisconnect()
{
    qDebug() << "OnDisconnect";
    _isConnected = false;
    Connect();
}

void NavPhotoInfoTcpListener::slot_OnConnect()
{
    qDebug() << "OnConnect";
    _connectionTimer->stop();
    _isConnected = true;

}

void NavPhotoInfoTcpListener::slot_OnConnectionTimer()
{
    if (_hostname != QString("none")){
        qDebug() << "Try connectToHost";
        _tcpSocket->connectToHost(_hostname, _hostport);
    }
}

