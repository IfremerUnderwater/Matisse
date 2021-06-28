#include "platform_component.h"

namespace system_tools {

PlatformComponent::PlatformComponent(QString _name) :
    m_name(_name)
{    
}

QString PlatformComponent::getName() const
{
    return m_name;
}

} // namespace system_tools


