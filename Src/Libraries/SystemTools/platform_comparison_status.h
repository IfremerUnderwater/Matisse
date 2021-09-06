#ifndef SYSTEM_TOOLS_PLATFORM_COMPARISON_STATUS_H_
#define SYSTEM_TOOLS_PLATFORM_COMPARISON_STATUS_H_

#include <QMap>
#include <QString>
#include <QStringList>

namespace system_tools {

enum eVersionCompare {
    SAME,
    OLDER,
    NEWER,
    DIFFERENT // when comparison is not possible and does not match
};

class PlatformElementCompare {
public:
    PlatformElementCompare() :
        m_found_in_remote_platform(true), m_found_in_local_platform(true), m_is_same_type(true), m_does_match(false)
    {}

    bool m_found_in_remote_platform;
    bool m_found_in_local_platform;
    bool m_is_same_type;
    bool m_does_match;
    eVersionCompare m_version_compare;
};

class PlatformComparisonStatus
{
public:
    PlatformComparisonStatus();
    ~PlatformComparisonStatus();

    bool doesMatch() const;
    void setDoesMatch(bool _does_match);

    PlatformElementCompare matisseCompared() const;
    void setMatisseCompared(const PlatformElementCompare &_matisse_compared);

    PlatformElementCompare osCompared() const;
    void setOsCompared(const PlatformElementCompare &_os_compared);

    QMap<QString, PlatformElementCompare> *componentsCompared() const;
    void addComponentCompared(QString _component_name, PlatformElementCompare _compared);

    QStringList getPlatformGapMessages() const;
    void addPlaformGapMessage(QString _message);

private:
    bool m_does_match;

    PlatformElementCompare m_matisse_compared;
    PlatformElementCompare m_os_compared;
    QMap<QString, PlatformElementCompare> *m_components_compared;

    QStringList m_platform_gap_messages;
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_COMPARISON_STATUS_H_
