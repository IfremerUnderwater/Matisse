#ifndef DIM2UDPLISTENER_H
#define DIM2UDPLISTENER_H
#include <QObject>
#include <QUdpSocket>

class Dim2UDPListener : public QObject
{
    Q_OBJECT

signals:
    void signal_newline(QString line);

public slots:
    void slot_configure(int port);
    void slot_receiveData();

public:
    Dim2UDPListener(QObject* parent=0);
    virtual ~Dim2UDPListener();

private:
    QUdpSocket *_udpSocket;
};

#endif // DIM2UDPLISTENER_H
