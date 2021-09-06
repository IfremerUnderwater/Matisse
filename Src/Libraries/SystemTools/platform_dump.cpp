#include "platform_dump.h"

namespace system_tools {

PlatformDump::PlatformDump() :
    m_components_info(NULL)
{

}

PlatformDump::~PlatformDump()
{
    if (m_components_info) {
        m_components_info->clear();
        delete m_components_info;
    }
}

QString PlatformDump::getOsName() const
{
    return m_os_name;
}

void PlatformDump::setOsName(const QString &_value)
{
    m_os_name = _value;
}
QString PlatformDump::getOsVersion() const
{
    return m_os_version;
}

void PlatformDump::setOsVersion(const QString &_value)
{
    m_os_version = _value;
}
QString PlatformDump::getEnvVariables() const
{
    return m_env_variables;
}

void PlatformDump::setEnvVariables(const QString &_value)
{
    m_env_variables = _value;
}

void PlatformDump::addComponentInfo(QString _component_name, QString _component_version)
{
    if (!m_components_info) {
        m_components_info = new QMap<QString,QString>();
    }

    m_components_info->insert(_component_name, _component_version);
}

QMap<QString, QString> *PlatformDump::getComponentsInfo() const
{
    return m_components_info;
}
QString PlatformDump::getMatisseVersion() const
{
    return m_matisse_version;
}

void PlatformDump::setMatisseVersion(const QString &_matisse_version)
{
    m_matisse_version = _matisse_version;
}

} // namespace system_tools




