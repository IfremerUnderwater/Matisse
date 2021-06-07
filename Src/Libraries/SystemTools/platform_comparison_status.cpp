#include "platform_comparison_status.h"

namespace system_tools {

PlatformComparisonStatus::PlatformComparisonStatus() :
    m_does_match(false),
    m_components_compared(NULL)
{
}

PlatformComparisonStatus::~PlatformComparisonStatus()
{
    if (m_components_compared) {
        m_components_compared->clear();
        delete m_components_compared;
    }
}

bool PlatformComparisonStatus::doesMatch() const
{
    return m_does_match;
}

void PlatformComparisonStatus::setDoesMatch(bool _does_match)
{
    m_does_match = _does_match;
}

PlatformElementCompare PlatformComparisonStatus::matisseCompared() const
{
    return m_matisse_compared;
}

void PlatformComparisonStatus::setMatisseCompared(const PlatformElementCompare &_matisse_compared)
{
    m_matisse_compared = _matisse_compared;
}

PlatformElementCompare PlatformComparisonStatus::osCompared() const
{
    return m_os_compared;
}

void PlatformComparisonStatus::setOsCompared(const PlatformElementCompare &_os_compared)
{
    m_os_compared = _os_compared;
}

QMap<QString, PlatformElementCompare> *PlatformComparisonStatus::componentsCompared() const
{
    return m_components_compared;
}

void PlatformComparisonStatus::addComponentCompared(QString _component_name, PlatformElementCompare _compared)
{
    if (!m_components_compared) {
        m_components_compared = new QMap<QString, PlatformElementCompare>();
    }

    m_components_compared->insert(_component_name, _compared);
}

QStringList PlatformComparisonStatus::getPlatformGapMessages() const
{
    return m_platform_gap_messages;
}

void PlatformComparisonStatus::addPlaformGapMessage(QString _message)
{
    m_platform_gap_messages.append(_message);
}

} // namespace system_tools





