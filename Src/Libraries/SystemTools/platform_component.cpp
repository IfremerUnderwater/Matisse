#include "platform_component.h"

namespace system_tools {

PlatformComponent::PlatformComponent(QString name) :
    _name(name)
{    
}

QString PlatformComponent::getName() const
{
    return _name;
}

} // namespace system_tools


