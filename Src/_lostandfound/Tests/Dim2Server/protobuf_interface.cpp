#include "protobuf_interface.h"


#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QStringList>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QMap>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QHostAddress>
#include <QString>
#include <QMutableListIterator>
#include <QByteArray>

#include <iostream>


ProtobufInterface::ProtobufInterface(unsigned int port) : QObject(),m_port(port)
{
    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    m_is_connected = false;

    m_tcp_server = new QTcpServer(this);

    connect(m_tcp_server, SIGNAL(newConnection()),
            this, SLOT(sl_OnNewConnection()));

    InitServer();

}

ProtobufInterface::~ProtobufInterface()
{
    if(m_tcp_server)
        delete m_tcp_server;
}



void
ProtobufInterface::sl_OnDisconnect()
{
    qDebug() << "One client has been disconnected";

    QMutableListIterator<QTcpSocket*> i(m_tcp_socket_list);
    while (i.hasNext()) {
        if (i.next()->state() != QAbstractSocket::ConnectedState)
            i.remove();
    }

    if (m_tcp_socket_list.size()==0){
        m_is_connected = false;
        emit si_Disconnected();
        qDebug() << "No connected client remaining...";
    }

}

void
ProtobufInterface::sl_OnDataReceived()
{

}

void ProtobufInterface::sl_OnNewConnection()
{
    // need to grab the socket
    m_tcp_socket_list.append(m_tcp_server->nextPendingConnection());

    connect(m_tcp_socket_list.last(),SIGNAL(disconnected()),this,SLOT(sl_OnDisconnect()));

    m_is_connected = true;
    emit si_Connected();
}

bool ProtobufInterface::InitServer()
{

    if (!m_tcp_server->listen(QHostAddress::Any, m_port)) {
        return false;
    }
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    // use the first non-localhost IPv4 address
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&
                ipAddressesList.at(i).toIPv4Address()) {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    std::cout << QString("The tcp server is running on\n\nIP: %1\nport: %2\n\n")
                 .arg(ipAddress).arg(m_tcp_server->serverPort()).toStdString();

    return true;

}
bool ProtobufInterface::IsConnected() const
{
    return m_is_connected;
}

void ProtobufInterface::sl_OnReceiveNavPhotoInfoMessage(NavPhotoInfoMessage msg)
{

    qDebug() << "Navigation message to send with latitude = " << msg.latitude();

    QByteArray serialMsg(msg.SerializeAsString().c_str(), msg.ByteSize());

    foreach (QTcpSocket* socket, m_tcp_socket_list){
        socket->write(serialMsg);
    }
}

