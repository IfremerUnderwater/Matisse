#ifndef SYSTEM_TOOLS_FILE_UTILS_H_
#define SYSTEM_TOOLS_FILE_UTILS_H_


#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QCryptographicHash>
#include <QByteArray>
#include <QDesktopServices>
#include <QThread>

#ifdef _MSC_VER
#include "quazip.h"
#include "quazipfile.h"
#include "quazipnewinfo.h"
#include "JlCompress.h"
#else
#include "quazip5/quazip.h"
#include "quazip5/quazipfile.h"
#include "quazip5/quazipnewinfo.h"
#include "quazip5/JlCompress.h"
#endif

#include "string_utils.h"

// This pattern contains a placeholder (%1) to insert the property name pattern (defined in StringUtils.h)
#define PROPERTY_DEFINITION_PATTERN     "%1\\s*\\=[^\\=]*"
#define TEMP_DIR_TEMPLATE_FORMAT        "'%1'yyyyMMdd'-'hhmmss'.'zzz"
#define TEMP_DIR_MAX_TRIALS             3
#define DEFAULT_TEMP_DIR_PREFIX         "matisse-"

namespace system_tools {


class FileUtils
{
public:
    static bool removeDir(const QString &_dir_name);
    static bool zipFiles(QString _zip_archive_path, QString _base_dir_path, QStringList _file_names, bool _append = false);
    static bool unzipFiles(QString _zip_archive_path, QString _dest_dir_path);
    static QStringList getZipEntries(QString _zip_archive_path);
    static bool areFilesIdentical(QString _file_path1, QString _file_path2);
    static QMap<QString,QString> readPropertiesFile(QString _prop_file_path);
    static bool createTempDirectory(QString &_temp_dir_path, QString _prefix = DEFAULT_TEMP_DIR_PREFIX);
    static void removeAllTempDirectories(QString _prefix = DEFAULT_TEMP_DIR_PREFIX);
    static quint64 dirSize(QString _dir_path, bool _include_subdirs = false);
    static quint32 fileCount(QString _dir_path, bool _include_subdirs = false);
    static QString resolveUnixPath(QString _file_spec);
};

} // namespace system_tools

#endif // SYSTEM_TOOLS_FILE_UTILS_H_
