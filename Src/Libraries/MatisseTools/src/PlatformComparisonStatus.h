#ifndef PLATFORMCOMPARISONSTATUS_H
#define PLATFORMCOMPARISONSTATUS_H

#include <QMap>
#include <QString>
#include <QStringList>

namespace MatisseTools {

enum VersionCompare {
    SAME,
    OLDER,
    NEWER,
    DIFFERENT // when comparison is not possible and does not match
};

class PlatformElementCompare {
public:
    PlatformElementCompare() :
        _foundInRemotePlatform(true), _foundInLocalPlatform(true), _isSameType(true), _doesMatch(false)
    {}

    bool _foundInRemotePlatform;
    bool _foundInLocalPlatform;
    bool _isSameType;
    bool _doesMatch;
    VersionCompare _versionCompare;
};

class PlatformComparisonStatus
{
public:
    PlatformComparisonStatus();
    ~PlatformComparisonStatus();

    bool doesMatch() const;
    void setDoesMatch(bool doesMatch);

    PlatformElementCompare matisseCompared() const;
    void setMatisseCompared(const PlatformElementCompare &matisseCompared);

    PlatformElementCompare osCompared() const;
    void setOsCompared(const PlatformElementCompare &osCompared);

    QMap<QString, PlatformElementCompare> *componentsCompared() const;
    void addComponentCompared(QString componentName, PlatformElementCompare compared);

    QStringList getPlatformGapMessages() const;
    void addPlaformGapMessage(QString message);

private:
    bool _doesMatch;

    PlatformElementCompare _matisseCompared;
    PlatformElementCompare _osCompared;
    QMap<QString, PlatformElementCompare> *_componentsCompared;

    QStringList _platformGapMessages;
};

}

#endif // PLATFORMCOMPARISONSTATUS_H
