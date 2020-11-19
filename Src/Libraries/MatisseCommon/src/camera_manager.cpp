#include "camera_manager.h"


using namespace MatisseCommon;

//// Constructor & Dectructor
CameraManager &CameraManager::instance()
{
    // create instance by lazy initialization
    // guaranteed to be destroyed
    static CameraManager instance;
    return instance;
}

