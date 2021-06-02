#ifndef MATISSE_CAMERA_MANAGER_H_
#define MATISSE_CAMERA_MANAGER_H_

#include "camera_info.h"
#include <QMap>
#include <QDir>
#include <QObject>

namespace matisse {

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

    bool deleteCameraByName(QString _camera_name);

    QDir camInfoDir() const;

private:
    CameraManager(QObject *_parent=nullptr);       // forbid create instance outside
    ~CameraManager();      // forbid to delete instance outside

    QMap<QString,CameraInfo> m_caminfo_map;
    QDir m_cam_info_dir;

signals:
    void signal_cameraListChanged();
};

} // namespace matisse

#endif // MATISSE_CAMERA_MANAGER_H_
