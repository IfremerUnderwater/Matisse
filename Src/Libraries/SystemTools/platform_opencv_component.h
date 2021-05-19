#ifndef PLATFORMOPENCVCOMPONENT_H
#define PLATFORMOPENCVCOMPONENT_H

#include "opencv2/core/version.hpp"

#include "PlatformComponent.h"

namespace MatisseTools {

class PlatformOpencvComponent : public PlatformComponent
{
public:
    PlatformOpencvComponent();

    virtual QString getVersionInfo();
};

}

#endif // PLATFORMOPENCVCOMPONENT_H
