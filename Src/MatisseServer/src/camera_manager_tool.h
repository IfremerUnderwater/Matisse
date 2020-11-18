#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <QDialog>

namespace Ui {
class CameraManager;
}

class CameraManagerTool : public QDialog
{
    Q_OBJECT

public:
    explicit CameraManagerTool(QWidget *parent = nullptr);
    ~CameraManagerTool();

private:
    Ui::CameraManager *ui;
};

#endif // CAMERA_MANAGER_H
