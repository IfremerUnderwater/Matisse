#ifndef PLATFORMCOMPONENT_H
#define PLATFORMCOMPONENT_H

#include <QString>

namespace MatisseTools {

class PlatformComponent
{
public:
    PlatformComponent(QString name);
    virtual QString getVersionInfo() = 0;

    QString getName() const;

private:
    QString _name;
};

}

#endif // DEPENDENCYINSPECTOR_H
