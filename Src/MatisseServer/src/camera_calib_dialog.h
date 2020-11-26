#ifndef CAM_CALIB_DIALOG_H
#define CAM_CALIB_DIALOG_H

#include <QDialog>

namespace Ui {
class CameraCalibDialog;
}

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

#endif // CAM_CALIB_DIALOG_H
