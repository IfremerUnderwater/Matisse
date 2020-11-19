#include "camera_manager.h"
#include <QStandardPaths>

using namespace MatisseCommon;

//// Constructor & Dectructor
CameraManager &CameraManager::instance()
{
    // create instance by lazy initialization
    // guaranteed to be destroyed
    static CameraManager instance;

    return instance;
}

void CameraManager::addCamera(CameraInfo _camera)
{
    m_caminfo_map[_camera.cameraName()] = _camera;

    QString camera_filename = m_cam_info_dir.absoluteFilePath(_camera.cameraName()+".yaml");
    _camera.writeToFile(camera_filename);
}

CameraManager::CameraManager()
{
    QDir config_path = QDir(QStandardPaths::locate(QStandardPaths::ConfigLocation, QString(), QStandardPaths::LocateDirectory));

    if (!config_path.exists("Matisse/CamInfoFiles"))
        config_path.mkpath("Matisse/CamInfoFiles");

    m_cam_info_dir = QDir(config_path.absoluteFilePath(QString("Matisse")+QDir::separator()+"CamInfoFiles"));
}

CameraManager::~CameraManager()
{

}

QDir CameraManager::camInfoDir() const
{
    return m_cam_info_dir;
}

