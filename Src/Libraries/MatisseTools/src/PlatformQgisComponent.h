#ifndef PLATFORMQGISCOMPONENT_H
#define PLATFORMQGISCOMPONENT_H

#include "qgis.h"
#include "PlatformComponent.h"

namespace MatisseTools {

class PlatformQGisComponent : public PlatformComponent
{
public:
    PlatformQGisComponent();

    virtual QString getVersionInfo();
};

}

#endif // PLATFORMQGISCOMPONENT_H
