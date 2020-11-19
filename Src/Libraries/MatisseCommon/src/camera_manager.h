#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "camera_info.h"
#include <QMap>
#include <QDir>

namespace MatisseCommon {

class CameraManager
{
public:
    static CameraManager& instance();

    CameraManager(CameraManager const&) = delete;        // Don't forget to disable copy
    void operator=(CameraManager const&) = delete;   // Don't forget to disable copy

    void addCamera(CameraInfo _camera);

    QDir camInfoDir() const;

private:
    CameraManager();       // forbid create instance outside
    ~CameraManager();      // forbid to delete instance outside

    QMap<QString,CameraInfo> m_caminfo_map;
    QDir m_cam_info_dir;
};

}

#endif // CAMERA_MANAGER_H
