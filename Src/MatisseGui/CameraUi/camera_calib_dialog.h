#ifndef MATISSE_CAMERA_CALIB_DIALOG_H_
#define MATISSE_CAMERA_CALIB_DIALOG_H_

#include <QDialog>

namespace Ui {
class CameraCalibDialog;
}

namespace matisse {

class CameraCalibDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraCalibDialog(QWidget *parent = nullptr);
    ~CameraCalibDialog();

private:
    Ui::CameraCalibDialog *ui;

    QString m_calib_data_path;

private slots:
    void slot_onCalibPathSelection();
    void slot_onCalibrateCameras();
};

}

#endif // MATISSE_CAMERA_CALIB_DIALOG_H_
