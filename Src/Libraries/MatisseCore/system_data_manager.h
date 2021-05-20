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
#include "MatisseRemoteServerSettings.h"
#include "platform_inspector.h"
#include "platform_dump.h"
#include "platform_comparator.h"
#include "platform_comparison_status.h"

namespace MatisseTools {

///
/// \brief The SystemDataManager class class is used for reading writing system data
///

class SystemDataManager
{
public:
    static SystemDataManager* instance();

    SystemDataManager(SystemDataManager const&) = delete;        // Disable copy
    void operator=(SystemDataManager const&) = delete;   // Disable copy

    void init(QString _bin_root_dir = ".");

    int port() const { return _port; }
    QString getUserDataPath() const { return _userDataPath; }
    QString getDllPath() const { return _dllPath; }
    QString getVersion() const { return _version; }
    QString getPlatformSummaryFilePath() const;
    QString getPlatformEnvDumpFilePath() const;
    QString getDataRootDir() const;
    QString getBinRootDir() const { return m_bin_root_dir; }
    QMap<QString, QString> getExternalTools() const;

    MatisseRemoteServerSettings *remoteServerSettings() { return m_remote_server_settings; }

    bool readMatisseSettings(QString filename);
    bool readMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writeMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writePlatformSummary();
    bool readRemotePlatformSummary(QString filename);
    bool writePlatformEnvDump();

    PlatformComparisonStatus *compareRemoteAndLocalPlatform();

private:
    SystemDataManager();
    ~SystemDataManager();      // forbid to delete instance outside

    void getPlatformDump();

    QString _userDataPath;
    QString _dataRootDir;
    QString m_bin_root_dir;
    QString _dllPath;
    QString _platformDumpPath;

    MatisseRemoteServerSettings *m_remote_server_settings;
    
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
