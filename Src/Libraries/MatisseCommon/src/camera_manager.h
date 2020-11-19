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

    void addCamera(CameraInfo _camera);

    QStringList cameraList();

    QDir camInfoDir() const;

private:
    CameraManager(QObject *_parent=nullptr);       // forbid create instance outside
    ~CameraManager();      // forbid to delete instance outside

    QMap<QString,CameraInfo> m_caminfo_map;
    QDir m_cam_info_dir;

signals:
    void sig_newCameraAdded();
};

}

#endif // CAMERA_MANAGER_H
