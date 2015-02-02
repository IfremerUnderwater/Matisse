#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QDebug>

#include "imageprocessor.h"
#include "sequenceprocessor.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QStringList captureDevices = m_VMRCap.EnumDevices();
    ui->cbxCam->addItems(captureDevices);
    ui->cbxCam->setCurrentIndex(0);
//    qDebug() << "Device list:";
//    foreach (QString captureDevice, captureDevices) {
//        qDebug() << captureDevice;
//    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnOpen_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     "",
                                                     tr("Images (*.bmp; *.jpg);;Videos (*.avi)"));
    qDebug() << "open" << fileName;

    if (proc != 0)
      delete proc;

    if (procseq != 0)
      delete procseq;

    if (fileName.endsWith(".avi")) {
        procseq = new SequenceProcessor(fileName);
        QStringList list = procseq->enumFilters();
        this->ui->textResult->clear();
        foreach (QString item, list) {
            this->ui->textResult->append(item);
        }
    }
    else {
        proc= new ImageProcessor(fileName);
    }


}

void MainWindow::on_btnProcess_clicked()
{
    if (proc != 0) {

          proc->execute();
          proc->display();
    }
}


void MainWindow::on_btnOK_clicked()
{

    QApplication::instance()->quit();

}


void MainWindow::on_btnInitCam_clicked()
{


    m_VMRCap.Init(ui->cbxCam->currentIndex(), (HWND)ui->videoLabel->winId(), 1920, 1080);


}

void MainWindow::on_btnGrab_clicked()
{
    DWORD dwSize = m_VMRCap.GrabFrame();

    if(dwSize>0)
        {
            qDebug() << "Grab image!" << dwSize;
            BYTE *pImage;
            m_VMRCap.GetFrame (&pImage);

            QImage *result = new QImage((const uchar*) pImage, 1920, 1080, 3*1920, QImage::Format_RGB888);
            const QSize size = ui->grabLabel->size();
            qDebug() << "Pix Width: "<< size.width();
            qDebug() << "QImage before Width: "<< result->size().width();
            QImage scaledImage = result->scaled(size, Qt::KeepAspectRatio);
            qDebug() << "QImage after Width: "<< scaledImage.size().width();

            const QPixmap pix = QPixmap::fromImage(scaledImage);

            ui->grabLabel->setPixmap(pix);
            //ui->grabLabel->setPixmap(pix.scaled(size,Qt::KeepAspectRatioByExpanding));


        }
}
