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

private:
    Ui::CameraManagerTool *ui;

private slots:
    void slot_saveCurrentCamera();
};

#endif // CAMERA_MANAGER_H
