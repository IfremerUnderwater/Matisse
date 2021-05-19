#ifndef PLATFORMCOMPARATOR_H
#define PLATFORMCOMPARATOR_H

#include <QObject>
#include <QStringList>
#include <QtDebug>

#include "platform_dump.h"
#include "platform_comparison_status.h"

namespace MatisseTools {

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

}

#endif // PLATFORMCOMPARATOR_H
