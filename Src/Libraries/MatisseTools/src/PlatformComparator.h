#ifndef PLATFORMCOMPARATOR_H
#define PLATFORMCOMPARATOR_H

#include <QObject>
#include <QStringList>
#include <QtDebug>

#include "PlatformDump.h"
#include "PlatformComparisonStatus.h"

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
