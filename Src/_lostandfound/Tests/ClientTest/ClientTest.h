#ifndef CLIENTTEST_H
#define CLIENTTEST_H

#include <QDialog>
#include <QTcpSocket>
#include <QByteArray>

namespace Ui {
class ClientTest;
}

class ClientTest : public QDialog
{
    Q_OBJECT
    
public:
    explicit ClientTest(QWidget *parent = 0);
    ~ClientTest();
    
protected slots:
    void slot_readyRead();
    void slot_connect();

private:
    Ui::ClientTest *ui;
    QTcpSocket _socket;

};

#endif // CLIENTTEST_H
