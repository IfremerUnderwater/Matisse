#include "platform_opencv_component.h"

namespace system_tools {

PlatformOpencvComponent::PlatformOpencvComponent() :
    PlatformComponent("OpenCV")
{

}

QString PlatformOpencvComponent::getVersionInfo()
{
    QString version = CV_VERSION;
    return version;
}

} // namespace system_tools


