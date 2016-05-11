#ifndef PLATFORMINSPECTOR_H
#define PLATFORMINSPECTOR_H

#include <QList>
#include <QProcess>
#include <QSysInfo>
#include <QtDebug>

#include "PlatformComponent.h"
#include "PlatformDump.h"

namespace MatisseTools {

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

}

#endif // PLATFORMINSPECTOR_H
