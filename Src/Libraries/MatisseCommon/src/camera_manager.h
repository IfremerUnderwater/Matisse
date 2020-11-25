#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "camera_info.h"
#include <QMap>
#include <QDir>
#include <QObject>

namespace MatisseCommon {

class CameraManager:public QObject
{
    Q_OBJECT
public:
    static CameraManager& instance();

    CameraManager(CameraManager const&) = delete;        // Don't forget to disable copy
    void operator=(CameraManager const&) = delete;   // Don't forget to disable copy

    void initializeFromDataBase();

    void deployDefaultCamerasToAppData();

    void addCamera(CameraInfo _camera);

    QStringList cameraList();
    CameraInfo cameraByName(QString _camera_name);
    QDir camInfoDir() const;

private:
    CameraManager(QObject *_parent=nullptr);       // forbid create instance outside
    ~CameraManager();      // forbid to delete instance outside

    QMap<QString,CameraInfo> m_caminfo_map;
    QDir m_cam_info_dir;

signals:
    void sigal_cameraListChanged();
};

}

#endif // CAMERA_MANAGER_H
