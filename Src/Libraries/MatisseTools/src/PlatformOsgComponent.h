#ifndef PLATFORMOSGCOMPONENT_H
#define PLATFORMOSGCOMPONENT_H


#include "PlatformComponent.h"


namespace MatisseTools {

class PlatformOsgComponent : public PlatformComponent
{
public:
    PlatformOsgComponent();
    virtual QString getVersionInfo();
};

}


#endif // PLATFORMOSGCOMPONENT_H
