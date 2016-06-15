#include "NetworkCheckDialog.h"
#include "ui_NetworkCheckDialog.h"

NetworkCheckDialog::NetworkCheckDialog(QWidget *parent) :
    QDialog(parent),
    _ui(new Ui::NetworkCheckDialog),
    _clientSocket(NULL)
{
    _ui->setupUi(this);

    _ui->_LE_udpPort->setValidator(new QRegExpValidator(QRegExp("\\d{2,}")));

    connect(_ui->_PB_connect, SIGNAL(clicked(bool)), this, SLOT(slot_connect(bool)));
    connect(_ui->_PB_reinit, SIGNAL(clicked(bool)), this, SLOT(slot_clearFrames()));
    connect(_ui->_PB_close, SIGNAL(clicked(bool)), this, SLOT(slot_quit()));
}

NetworkCheckDialog::~NetworkCheckDialog()
{
    clearSocket();
    delete _ui;
}

void NetworkCheckDialog::slot_connect(bool checked)
{
    if (!checked) {
        clearSocket();
        return;
    }

    QString udpPortText = _ui->_LE_udpPort->text();
    bool isPortValid = false;
    int udpPort = udpPortText.toInt(&isPortValid);

    if (!isPortValid) {
        QMessageBox::critical(this, tr("Connexion impossible..."), tr("Le port saisi est invalide."));
        return;
    }

    qDebug() << "Connecting to UDP port " << udpPortText;

    _clientSocket = new QUdpSocket(this);
    bool connected = _clientSocket->bind(udpPort);
    if (!connected) {
        QMessageBox::critical(this, tr("Connexion impossible..."), tr("Impossible de se connecter sur le port selectionne."));
        clearSocket();
        return;
    }

    connect(_clientSocket, SIGNAL(readyRead()), this, SLOT(slot_readPendingDatagrams()));
    qDebug() << "Connected !";
}

void NetworkCheckDialog::slot_clearFrames()
{
    _ui->_TB_frames->clear();
}

void NetworkCheckDialog::slot_quit()
{
    clearSocket();
    accept();
}

void NetworkCheckDialog::slot_readPendingDatagrams()
{
    while(_clientSocket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(_clientSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        _clientSocket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
        _ui->_TB_frames->append(buffer.data());
    }
}

void NetworkCheckDialog::clearSocket()
{
    if (_clientSocket) {
        if (_clientSocket->isValid()) {
            qDebug() << "Disconnecting from UDP server...";
            _clientSocket->disconnectFromHost();
            qDebug() << "Disconnected";
        }

        delete _clientSocket;
        _clientSocket = NULL;
    }
}
