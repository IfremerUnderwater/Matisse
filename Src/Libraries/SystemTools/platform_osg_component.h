#ifndef SYSTEM_TOOLS_PLATFORM_OSG_COMPONENT_H_
#define SYSTEM_TOOLS_PLATFORM_OSG_COMPONENT_H_


#include "platform_component.h"


namespace system_tools {

class PlatformOsgComponent : public PlatformComponent
{
public:
    PlatformOsgComponent();
    virtual QString getVersionInfo();
};

} // namespace system_tools


#endif // SYSTEM_TOOLS_PLATFORM_OSG_COMPONENT_H_
