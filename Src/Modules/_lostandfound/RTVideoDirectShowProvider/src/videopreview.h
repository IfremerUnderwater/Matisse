#ifndef VIDEOPREVIEW_H
#define VIDEOPREVIEW_H

#include "VMR_Capture.h"
#include <QDialog>

namespace Ui {
class VideoPreview;
}

class VideoPreview : public QDialog
{
    Q_OBJECT

public:
    explicit VideoPreview( QWidget *parent, CVMR_Capture *cmr );
    ~VideoPreview();
    HWND getVideoWidget();
    int getWidth();
    int getHeight();

private slots:
    void on_pushButton_clicked();
    void on_toolButton_clicked();

private:
    Ui::VideoPreview *ui;
    CVMR_Capture *_captureV;


};

#endif // VIDEOPREVIEW_H
