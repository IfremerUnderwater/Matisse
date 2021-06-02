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
    void setOsName(const QString &value);

    QString getOsVersion() const;
    void setOsVersion(const QString &value);

    QString getEnvVariables() const;
    void setEnvVariables(const QString &value);

    void addComponentInfo(QString componentName, QString componentVersion);

    QMap<QString, QString> *getComponentsInfo() const;

    QString getMatisseVersion() const;
    void setMatisseVersion(const QString &matisseVersion);

private:
    QMap<QString, QString> *_componentsInfo;
    QString _matisseVersion;
    QString _osName;
    QString _osVersion;
    QString _envVariables;
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_DUMP_H_
