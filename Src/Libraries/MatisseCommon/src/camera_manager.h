#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H


namespace MatisseCommon {

class CameraManager
{
public:
    static CameraManager& instance();

    CameraManager(CameraManager const&) = delete;        // Don't forget to disable copy
    void operator=(CameraManager const&) = delete;   // Don't forget to disable copy


private:
    CameraManager();       // forbid create instance outside
    ~CameraManager();      // forbid to delete instance outside
    double m_dpi;
};

}

#endif // CAMERA_MANAGER_H
