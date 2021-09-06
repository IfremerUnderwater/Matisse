#ifndef MATISSE_SYSTEM_DATA_MANAGER_H_
#define MATISSE_SYSTEM_DATA_MANAGER_H_

#include <QString>
#include <QDate>
#include <QFile>
#include <QFileInfo>
#include <QtXml>
#include <QXmlAttributes>
#include <QtDebug>

#include "matisse_preferences.h"
#include "matisse_remote_server_settings.h"
#include "platform_inspector.h"
#include "platform_dump.h"
#include "platform_comparator.h"
#include "platform_comparison_status.h"

using namespace system_tools;

namespace matisse {

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

    int port() const { return m_port; }
    QString getUserDataPath() const { return m_user_data_path; }
    QString getDllPath() const { return m_dll_path; }
    QString getVersion() const { return m_version; }
    QString getPlatformSummaryFilePath() const;
    QString getPlatformEnvDumpFilePath() const;
    QString getDataRootDir() const;
    QString getBinRootDir() const { return m_bin_root_dir; }
    QMap<QString, QString> getExternalTools() const;

    MatisseRemoteServerSettings *remoteServerSettings() { return m_remote_server_settings; }

    bool readMatisseSettings(QString _filename);
    bool readMatissePreferences(QString _filename, MatissePreferences &_prefs);
    bool writeMatissePreferences(QString _filename, MatissePreferences &_prefs);
    bool writePlatformSummary();
    bool readRemotePlatformSummary(QString _filename);
    bool writePlatformEnvDump();

    PlatformComparisonStatus *compareRemoteAndLocalPlatform();

private:
    SystemDataManager();
    ~SystemDataManager();      // forbid to delete instance outside

    void getPlatformDump();

    QString m_user_data_path;
    QString m_data_root_dir;
    QString m_bin_root_dir;
    QString m_dll_path;
    QString m_platform_dump_path;

    MatisseRemoteServerSettings *m_remote_server_settings;
    
    int m_port;
    QString m_version;
    QString m_platform_summary_file_path;
    QString m_platform_env_dump_file_path;
    QMap<QString,QString> m_external_tools;
    PlatformInspector m_platform_inspector;
    PlatformDump *m_platform_dump;
    PlatformDump *m_remote_platform_dump;
    PlatformComparator m_platform_comparator;
};

} // namespace matisse

#endif // MATISSE_SYSTEM_DATA_MANAGER_H_
