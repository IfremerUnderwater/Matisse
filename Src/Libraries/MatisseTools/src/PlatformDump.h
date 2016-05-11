#ifndef PLATFORMDUMP_H
#define PLATFORMDUMP_H

#include <QString>
#include <QMap>

namespace MatisseTools {

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

}

#endif // PLATFORMDUMP_H
