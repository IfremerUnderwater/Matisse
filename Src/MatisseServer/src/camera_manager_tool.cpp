#include "camera_manager_tool.h"
#include "ui_camera_manager_tool.h"

CameraManagerTool::CameraManagerTool(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraManager)
{
    ui->setupUi(this);
}

CameraManagerTool::~CameraManagerTool()
{
    delete ui;
}
