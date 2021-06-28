#ifndef MATISSE_CAMERA_MANAGER_TOOL_H_
#define MATISSE_CAMERA_MANAGER_TOOL_H_

#include <QDialog>

namespace Ui {
class CameraManagerTool;
}

namespace matisse {

class CameraManagerTool : public QDialog
{
    Q_OBJECT

public:
    explicit CameraManagerTool(QWidget *_parent = nullptr);
    ~CameraManagerTool();

public slots:
    void sl_refreshCameraList();

private:
    Ui::CameraManagerTool *m_ui;

private slots:
    void sl_saveCurrentCamera();
    void sl_deleteCurrentCamera();
    void sl_cameraSelected(QString _selected_item);
    void sl_distModelChanged(int _dist_model);
};

} // namespace matisse

#endif // CAMERA_MANAGER_H
