#ifndef SYSTEM_TOOLS_PLATFORM_COMPONENT_H_
#define SYSTEM_TOOLS_PLATFORM_COMPONENT_H_

#include <QString>

namespace system_tools {

class PlatformComponent
{
public:
    PlatformComponent(QString _name);
    virtual ~PlatformComponent(){}
    virtual QString getVersionInfo() = 0;

    QString getName() const;

private:
    QString m_name;
};

} // namespace system_tools

#endif // DEPENDENCYINSPECTOR_H
