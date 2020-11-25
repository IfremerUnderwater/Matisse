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

void CameraManager::initializeFromDataBase()
{
    QStringList cam_info_file_list = m_cam_info_dir.entryList(QStringList()<<"*.yaml");

    for (int i=0; i<cam_info_file_list.size(); i++)
    {
        CameraInfo current_cam_info;
        QString current_cam_info_file = m_cam_info_dir.absoluteFilePath(cam_info_file_list[i]);
        if(current_cam_info.readFromFile(current_cam_info_file))
            addCamera(current_cam_info);
    }
}

void CameraManager::deployDefaultCamerasToAppData()
{
    QDir default_cam_info_dir("./cameras");
    QStringList default_cam_info_file_list = default_cam_info_dir.entryList(QStringList()<<"*.yaml");

    for(int i=0; i<default_cam_info_file_list.size(); i++)
    {
        QString current_src_camera_file = default_cam_info_dir.absoluteFilePath(default_cam_info_file_list[i]);
        QString current_dest_camera_file = m_cam_info_dir.absoluteFilePath(default_cam_info_file_list[i]);
        if (QFile::exists(current_dest_camera_file))
        {
            QFile::remove(current_dest_camera_file);
        }

        QFile::copy(current_src_camera_file, current_dest_camera_file);
    }


}

void CameraManager::addCamera(CameraInfo _camera)
{
    m_caminfo_map[_camera.cameraName()] = _camera;

    QString camera_filename = m_cam_info_dir.absoluteFilePath(_camera.cameraName()+".yaml");

    if(_camera.writeToFile(camera_filename))
        emit sigal_cameraListChanged();
}

QStringList CameraManager::cameraList()
{
    return m_caminfo_map.keys();
}

CameraInfo CameraManager::cameraByName(QString _camera_name)
{
    return  m_caminfo_map[_camera_name];
}

CameraManager::CameraManager(QObject *_parent):QObject(_parent)
{
    QDir config_path = QDir(QStandardPaths::locate(QStandardPaths::AppDataLocation, QString(), QStandardPaths::LocateDirectory));

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

