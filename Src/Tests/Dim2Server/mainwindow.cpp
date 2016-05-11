#include <QFileDialog>


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Dim2FileReader.h"
#include "protobuf_interface.h"

#define _USE_MATH_DEFINES
#include "math.h"

#define D2R (3.14159265358979323846 / 180.0)

using namespace MatisseCommon;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _udpSocket(NULL),
    _reader(NULL),
    _isTcp(false)
{
    ui->setupUi(this);
    ui->lineEditPort->setText("5600");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initServer()
{
    bool isOk;
    _port = ui->lineEditPort->text().toInt(&isOk);
    if (!isOk) {
        qDebug() << "Erreur de port " << ui->lineEditPort->text();
        return;
    }

    if (ui->comboBox->currentText() == QString("Dim2UDPServer")){
        _udpSocket = new QUdpSocket();
        _udpAddress = new QHostAddress(QHostAddress::Broadcast);
        _isTcp = false;
    }else if (ui->comboBox->currentText() == QString("NavPhotoInfoServer")){

        _protoInterface = new ProtobufInterface(_port);
        _isTcp = true;
    }

}

void MainWindow::on_pushButtonFile_clicked()
{
    QString retFile;
    retFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Choix d'un fichier..."), "*.dim2");
    if (!retFile.isEmpty()) {
        QFileInfo fileInfo(retFile);
        retFile = fileInfo.absoluteFilePath();
        ui->lineEditFile->setText(retFile);
    }
}


void MainWindow::on_buttonStart_toggled(bool checked)
{

    initServer();

    if( checked )  /// Activation de l'emission
    {
        QString fileName =  ui->lineEditFile->text();
        QFile *fi = new QFile(fileName);
        if (!fi->exists()) {
            qDebug() << tr("Fichier non trouvé :") << fileName;
            return;
        }

        if (!fi->open(QIODevice::ReadOnly)) {
            qDebug() << "File not opened" << fileName;
            return;
        }
        _reader = new QTextStream(fi);


        int periode = ui->sB_Periode->value();
        _indTimer  = startTimer( periode*1000 );
    }
    else            /// Desactivation de l'emission
    {
        killTimer(_indTimer);
        if( !_reader )
            delete( _reader );
    }

}

void MainWindow::timerEvent(QTimerEvent *event )
{
    qDebug() << "timerEvent";

    Q_UNUSED(event);

    QString newLine = _reader->readLine();
    if(!_reader->atEnd())
    {
        if (!_isTcp){

        qDebug() << " load image " << newLine;
        QHostAddress address(QHostAddress::Broadcast);
        _udpSocket->writeDatagram(newLine.toAscii(), address, _port);
        _udpSocket->flush();
        }else{
            Dim2 dim2(newLine);
            NavPhotoInfoMessage msg;
            msg.set_altitude(dim2.altitude());
            msg.set_depth(dim2.depth());
            msg.set_latitude(dim2.latitude());
            msg.set_longitude(dim2.longitude());
            msg.set_pan(dim2.mainPan());
            msg.set_photopath(dim2.filename().toStdString().c_str(),dim2.filename().toStdString().size());
            msg.set_pitch(D2R*dim2.pitch());
            msg.set_roll(D2R*dim2.roll());
            msg.set_tilt(dim2.mainTilt());
            msg.set_yaw(D2R*dim2.yaw());
            _protoInterface->sl_OnReceiveNavPhotoInfoMessage(msg);
        }
    }
}
