#include "ClientTest.h"
#include "ui_ClientTest.h"

ClientTest::ClientTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClientTest)
{
    ui->setupUi(this);
    connect(ui->_PB_connect, SIGNAL(clicked()), this, SLOT(slot_connect()));
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(slot_readyRead()));
    ui->_LE_host->setText("192.168.0.191");
    ui->_LE_port->setText("54637");
}

ClientTest::~ClientTest()
{
    delete ui;
}

void ClientTest::slot_readyRead()
{
    QByteArray datas = _socket.readAll();
    foreach(QByteArray data, datas.split(';')){
        QString message(data.simplified());
        if (message != "") {
        ui->_LW_processors->addItem(message);
        }
    }
}

void ClientTest::slot_connect() {

    QString address = ui->_LE_host->text();
    quint16 port = ui->_LE_port->text().toInt();
    _socket.connectToHost(address, port);

}
