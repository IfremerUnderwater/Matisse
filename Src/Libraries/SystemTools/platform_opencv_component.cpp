#include "platform_opencv_component.h"

using namespace MatisseTools;

PlatformOpencvComponent::PlatformOpencvComponent() :
    PlatformComponent("OpenCV")
{

}

QString PlatformOpencvComponent::getVersionInfo()
{
    QString version = CV_VERSION;
    return version;
}


