#ifndef PLATFORMOSGCOMPONENT_H
#define PLATFORMOSGCOMPONENT_H

#ifdef WITH_OSG

#include "PlatformComponent.h"

#include "osg/Version"

namespace MatisseTools {

class PlatformOsgComponent : public PlatformComponent
{
public:
    PlatformOsgComponent();
    virtual QString getVersionInfo();
};

}

#endif

#endif // PLATFORMOSGCOMPONENT_H
