#ifndef SYSTEM_TOOLS_PLATFORM_INSPECTOR_H_
#define SYSTEM_TOOLS_PLATFORM_INSPECTOR_H_

#include <QList>
#include <QProcess>
#include <QSysInfo>
#include <QtDebug>

#include "platform_component.h"
#include "platform_dump.h"

namespace system_tools {

class PlatformInspector
{
public:
    PlatformInspector();
    ~PlatformInspector();
    void init();
    void inspect();

    PlatformDump *getDump() const;
    void inspectOsAndEnvVariables();

private:
    QList<PlatformComponent*> _components;
    PlatformDump *_dump;
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_INSPECTOR_H_
