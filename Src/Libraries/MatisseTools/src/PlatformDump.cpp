#include "PlatformDump.h"

using namespace MatisseTools;

PlatformDump::PlatformDump() :
    _componentsInfo(NULL)
{

}

PlatformDump::~PlatformDump()
{
    if (_componentsInfo) {
        _componentsInfo->clear();
        delete _componentsInfo;
    }
}

QString PlatformDump::getOsName() const
{
    return _osName;
}

void PlatformDump::setOsName(const QString &value)
{
    _osName = value;
}
QString PlatformDump::getOsVersion() const
{
    return _osVersion;
}

void PlatformDump::setOsVersion(const QString &value)
{
    _osVersion = value;
}
QString PlatformDump::getEnvVariables() const
{
    return _envVariables;
}

void PlatformDump::setEnvVariables(const QString &value)
{
    _envVariables = value;
}

void PlatformDump::addComponentInfo(QString componentName, QString componentVersion)
{
    if (!_componentsInfo) {
        _componentsInfo = new QMap<QString,QString>();
    }

    _componentsInfo->insert(componentName, componentVersion);
}

QMap<QString, QString> *PlatformDump::getComponentsInfo() const
{
    return _componentsInfo;
}
QString PlatformDump::getMatisseVersion() const
{
    return _matisseVersion;
}

void PlatformDump::setMatisseVersion(const QString &matisseVersion)
{
    _matisseVersion = matisseVersion;
}






