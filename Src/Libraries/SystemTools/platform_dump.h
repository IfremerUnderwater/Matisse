#ifndef SYSTEM_TOOLS_PLATFORM_DUMP_H_
#define SYSTEM_TOOLS_PLATFORM_DUMP_H_

#include <QString>
#include <QMap>

namespace system_tools {

class PlatformDump
{
public:
    PlatformDump();
    ~PlatformDump();

    QString getOsName() const;
    void setOsName(const QString &_value);

    QString getOsVersion() const;
    void setOsVersion(const QString &_value);

    QString getEnvVariables() const;
    void setEnvVariables(const QString &_value);

    void addComponentInfo(QString _component_name, QString _component_version);

    QMap<QString, QString> *getComponentsInfo() const;

    QString getMatisseVersion() const;
    void setMatisseVersion(const QString &_matisse_version);

private:
    QMap<QString, QString> *m_components_info;
    QString m_matisse_version;
    QString m_os_name;
    QString m_os_version;
    QString m_env_variables;
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_DUMP_H_
