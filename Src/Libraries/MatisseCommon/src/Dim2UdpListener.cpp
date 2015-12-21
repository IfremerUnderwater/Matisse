#include "Dim2UdpListener.h"

Dim2UDPListener::Dim2UDPListener(QObject *parent)
    : QObject(parent),
      _udpSocket(NULL)
{
}

Dim2UDPListener::~Dim2UDPListener()
{
    qDebug() << "Destroy Dim2UDPListener!";
    if (_udpSocket) {
        _udpSocket->deleteLater();
    }
}

void Dim2UDPListener::slot_configure(int port)
{
    _udpSocket = new QUdpSocket(parent());
    _udpSocket->bind(port);
    connect(_udpSocket, SIGNAL(readyRead()), this, SLOT(slot_receiveData()));
}

void Dim2UDPListener::slot_receiveData()
{
    qDebug() << "Receiving!";
    QByteArray datagram;
    do {
        datagram.resize(_udpSocket->pendingDatagramSize());
        _udpSocket->readDatagram(datagram.data(), datagram.size());

        QString dim2String(datagram);
        //qDebug() << "Receive: " << dim2String;
        emit signal_newline(dim2String);
    } while (_udpSocket->hasPendingDatagrams());
}

