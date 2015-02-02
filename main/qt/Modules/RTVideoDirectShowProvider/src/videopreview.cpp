#include "videopreview.h"
#include "ui_videopreview.h"

VideoPreview::VideoPreview(QWidget *parent,  CVMR_Capture *cmr) :
    QDialog(parent),
    ui(new Ui::VideoPreview),
    _captureV(cmr)
{
    ui->setupUi(this);
}

VideoPreview::~VideoPreview()
{
    delete ui;
}

void VideoPreview::on_pushButton_clicked()
{
    close();
}

void VideoPreview::on_toolButton_clicked()
{
    _captureV->showFFdShowProperties((HWND)this->winId());
}

HWND  VideoPreview::getVideoWidget(){
    return (HWND)ui->videoWidget->winId();
}

int VideoPreview::getWidth()
{
    return ui->videoWidget->width();
}

int VideoPreview::getHeight()
{
    return ui->videoWidget->height();
}
