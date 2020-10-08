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
    SystemDataManager(QString _bin_root_dir = ".");

    int port() const { return _port; }
    QString getUserDataPath() const { return _userDataPath; }
    QString getDllPath() const { return _dllPath; }
    QString getVersion() const { return _version; }
    QString getPlatformSummaryFilePath() const;
    QString getPlatformEnvDumpFilePath() const;
    QString getDataRootDir() const;
    QString getBinRootDir() const { return m_bin_root_dir; }
    QMap<QString, QString> getExternalTools() const;

    QString defaultRemoteCommandServer() const { return m_default_remote_command_server; }
    QString defaultRemoteFileServer() const { return m_default_remote_file_server; }
    QString defaultRemoteQueueName() const { return m_default_remote_queue_name; }
    int defaultRemoteNbOfCpus() const { return m_default_remote_nb_of_cpus; }
    QString getDefaultRemoteDataPath() const { return m_default_remote_data_path; }
    QString getDefaultRemoteResultPath() const { return m_default_remote_result_path; }

    MatisseRemoteServerSettings *remoteServerSettings() { return m_remote_server_settings; }

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
    QString m_bin_root_dir;
    QString _dllPath;
    QString _platformDumpPath;

    MatisseRemoteServerSettings *m_remote_server_settings;
    
    QString m_default_remote_command_server;
    QString m_default_remote_file_server;
    QString m_default_remote_queue_name;
    int m_default_remote_nb_of_cpus = 1;
    QString m_default_remote_data_path;
    QString m_default_remote_result_path;
    
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
