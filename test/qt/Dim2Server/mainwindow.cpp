#include <QFileDialog>


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "Dim2FileReader.h"

using namespace MatisseCommon;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _socket(NULL),
    _reader(NULL)
{
    ui->setupUi(this);
    ui->lineEditPort->setText("7777");
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

    _socket = new QUdpSocket();
    _address = new QHostAddress(QHostAddress::Broadcast);

}

void MainWindow::on_pushButtonFile_clicked()
{
    QString retFile;
    retFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Choix d'un fichier..."), "*.dim2");
    if (!retFile.isEmpty()) {
        QFileInfo fileInfo(retFile);
        retFile = fileInfo.absoluteFilePath();
        ui->lineEditFile->setText(retFile);
        initServer();
    }
}


void MainWindow::on_buttonStart_toggled(bool checked)
{

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
       qDebug() << " load image " << newLine;
       QHostAddress address(QHostAddress::Broadcast);
       _socket->writeDatagram(newLine.toAscii(), address, _port);
       _socket->flush();
    }
}
