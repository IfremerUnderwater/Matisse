#ifndef CAMERA_MANAGER_TOOL_H
#define CAMERA_MANAGER_TOOL_H

#include <QDialog>

namespace Ui {
class CameraManagerTool;
}

class CameraManagerTool : public QDialog
{
    Q_OBJECT

public:
    explicit CameraManagerTool(QWidget *parent = nullptr);
    ~CameraManagerTool();

public slots:
    void slot_refreshCameraList();

private:
    Ui::CameraManagerTool *ui;

private slots:
    void slot_saveCurrentCamera();
    void slot_cameraSelected(QString _selected_item);
    void slot_distModelChanged(int _dist_model);
};

#endif // CAMERA_MANAGER_H
