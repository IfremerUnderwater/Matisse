#ifndef SYSTEM_TOOLS_PLATFORM_OPENCV_COMPONENT_H_
#define SYSTEM_TOOLS_PLATFORM_OPENCV_COMPONENT_H_

#include "opencv2/core/version.hpp"

#include "platform_component.h"

namespace system_tools {

class PlatformOpencvComponent : public PlatformComponent
{
public:
    PlatformOpencvComponent();

    virtual QString getVersionInfo();
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_OPENCV_COMPONENT_H_
