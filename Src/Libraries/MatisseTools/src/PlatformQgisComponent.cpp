#include "PlatformQgisComponent.h"

using namespace MatisseTools;

PlatformQGisComponent::PlatformQGisComponent() :
    PlatformComponent("QGis")
{    
}

QString PlatformQGisComponent::getVersionInfo()
{
    QString version = Qgis::QGIS_VERSION;
    return version;
}

