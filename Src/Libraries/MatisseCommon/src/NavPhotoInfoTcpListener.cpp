#include "NavPhotoInfoTcpListener.h"
#include <QThread>

NavPhotoInfoTcpListener::NavPhotoInfoTcpListener() : QObject(), _connectionAllowed(false)
{
    _connectionTimer = new QTimer(this);
    _tcpSocket = new QTcpSocket(this);

    _isConnected = false;
    _hostname = QString("none");

    connect(_connectionTimer, SIGNAL(timeout()), this, SLOT(slot_OnConnectionTimer()));

    connect(_tcpSocket, SIGNAL(disconnected()), this, SLOT(slot_OnDisconnect()));
    connect(_tcpSocket, SIGNAL(connected()), this, SLOT(slot_OnConnect()));
    connect(_tcpSocket, SIGNAL(readyRead()), this, SLOT(slot_OnDataReceived()));

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
    _connectionAllowed = true;
    Connect();
}

void NavPhotoInfoTcpListener::Connect()
{
    qDebug() << " NavPhotoInfo Thread = " << QThread::currentThread();
    if(_connectionAllowed){
        _tcpSocket->connectToHost(_hostname, _hostport);
        _connectionTimer->start();
    }

}

void NavPhotoInfoTcpListener::slot_disconnect(){
    _connectionTimer->stop();
    _connectionAllowed = false;
    _tcpSocket->disconnectFromHost();
    _isConnected = false;
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

