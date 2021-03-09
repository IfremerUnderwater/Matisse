#include "PlatformComponent.h"

using namespace MatisseTools;

PlatformComponent::PlatformComponent(QString name) :
    _name(name)
{    
}

QString PlatformComponent::getName() const
{
    return _name;
}


