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

    PlatformComparisonStatus *compare(PlatformDump *localPlatform, PlatformDump *remotePlatform);

private:
    PlatformComparisonStatus *_status;
    void compareElementVersions(QString localVersion, QString remoteVersion, PlatformElementCompare &versionCompare);
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_PLATFORM_COMPARATOR_H_
