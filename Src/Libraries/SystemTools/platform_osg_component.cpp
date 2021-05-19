#include "platform_osg_component.h"
#include <OpenThreads/Version>
#include <osg/Version>

using namespace MatisseTools;

PlatformOsgComponent::PlatformOsgComponent() :
    PlatformComponent("OpenSceneGraph")
{
}

QString MatisseTools::PlatformOsgComponent::getVersionInfo()
{
    QString version = osgGetVersion();
    return version;
}
