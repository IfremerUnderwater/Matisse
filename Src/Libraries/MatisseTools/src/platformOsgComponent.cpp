#include "PlatformOsgComponent.h"

#ifdef WITH_OSG

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

#endif
