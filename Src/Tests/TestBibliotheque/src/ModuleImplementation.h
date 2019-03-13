#ifndef MODULEIMPLEMENTATION_H
#define MODULEIMPLEMENTATION_H

#include <QList>
#include <QVariant>
#include <QtDebug>

#include "ModuleInterface.h"

class ModuleImplementation : public ModuleInterface
{
    Q_OBJECT
    Q_INTERFACES(ModuleInterface)

public:
    ModuleImplementation();

    virtual void init();
    virtual void run();
//    virtual ModuleImplementation * newInstance();
};

#endif // MODULEIMPLEMENTATION_H
