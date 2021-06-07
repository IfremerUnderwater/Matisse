#ifndef SYSTEM_TOOLS_PLATFORM_COMPARATOR_H_
#define SYSTEM_TOOLS_PLATFORM_COMPARATOR_H_

#include <QObject>
#include <QStringList>
#include <QtDebug>

#include "platform_dump.h"
#include "platform_comparison_status.h"

namespace system_tools {

class PlatformComparator : public QObject
{
public:
    PlatformComparator();
    ~PlatformComparator();

    PlatformComparisonStatus *compare(PlatformDump *_local_platform, PlatformDump *_remote_platform);

private:
    void compareElementVersions(QString _local_version, QString _remote_version, PlatformElementCompare &_version_compare);

    PlatformComparisonStatus *m_status;
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_COMPARATOR_H_
