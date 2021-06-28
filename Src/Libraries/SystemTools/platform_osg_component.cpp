#include "platform_osg_component.h"
#include <OpenThreads/Version>
#include <osg/Version>

namespace system_tools {

PlatformOsgComponent::PlatformOsgComponent() :
    PlatformComponent("OpenSceneGraph")
{
}

QString PlatformOsgComponent::getVersionInfo()
{
    QString version = osgGetVersion();
    return version;
}

} // namespace system_tools
