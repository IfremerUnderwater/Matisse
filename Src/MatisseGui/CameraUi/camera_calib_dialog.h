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
    explicit CameraCalibDialog(QWidget *_parent = nullptr);
    ~CameraCalibDialog();

private:
    Ui::CameraCalibDialog *m_ui;

    QString m_calib_data_path;

private slots:
    void sl_onCalibPathSelection();
    void sl_onCalibrateCameras();
};

}

#endif // MATISSE_CAMERA_CALIB_DIALOG_H_
