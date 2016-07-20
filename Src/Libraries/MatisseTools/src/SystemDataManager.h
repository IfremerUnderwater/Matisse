#ifndef SYSTEMDATAMANAGER_H
#define SYSTEMDATAMANAGER_H

#include <QString>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QtXml>
#include <QXmlAttributes>
#include <QtDebug>

#include "MatissePreferences.h"
#include "PlatformInspector.h"
#include "PlatformDump.h"
#include "PlatformComparator.h"
#include "PlatformComparisonStatus.h"

namespace MatisseTools {

///
/// \brief The SystemDataManager class class is used for reading writing system data
///

class SystemDataManager
{
public:
    SystemDataManager();

    int port() const { return _port; }
    QString getUserDataPath() const { return _userDataPath; }
    QString getDllPath() const { return _dllPath; }
    QString getVersion() const { return _version; }
    QString getPlatformSummaryFilePath() const;
    QString getPlatformEnvDumpFilePath() const;
    QString getDataRootDir() const;
    QMap<QString, QString> getExternalTools() const;

    bool readMatisseSettings(QString filename);
    bool readMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writeMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writePlatformSummary();
    bool readRemotePlatformSummary(QString filename);
    bool writePlatformEnvDump();

    PlatformComparisonStatus *compareRemoteAndLocalPlatform();


private:
    void getPlatformDump();

    QString _userDataPath;
    QString _dataRootDir;
    QString _dllPath;
    QString _platformDumpPath;
    int _port;
    QString _version;
    QString _platformSummaryFilePath;
    QString _platformEnvDumpFilePath;
    QMap<QString,QString> _externalTools;
    PlatformInspector _platformInspector;
    PlatformDump *_platformDump;
    PlatformDump *_remotePlatformDump;
    PlatformComparator _platformComparator;
};

}

#endif // SYSTEMDATAMANAGER_H
