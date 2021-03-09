#ifndef FILE_UTILS_H
#define FILE_UTILS_H



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

#include "StringUtils.h"

// This pattern contains a placeholder (%1) to insert the property name pattern (defined in StringUtils.h)
#define PROPERTY_DEFINITION_PATTERN     "%1\\s*\\=[^\\=]*"
#define TEMP_DIR_TEMPLATE_FORMAT        "'%1'yyyyMMdd'-'hhmmss'.'zzz"
#define TEMP_DIR_MAX_TRIALS             3
#define DEFAULT_TEMP_DIR_PREFIX         "matisse-"

namespace MatisseCommon {


class FileUtils
{
public:
    static bool removeDir(const QString &dirName);
    static bool zipFiles(QString zipArchivePath, QString baseDirPath, QStringList fileNames, bool append = false);
    static bool unzipFiles(QString zipArchivePath, QString destDirPath);
    static QStringList getZipEntries(QString zipArchivePath);
    static bool areFilesIdentical(QString filePath1, QString filePath2);
    static QMap<QString,QString> readPropertiesFile(QString propFilePath);
    static bool createTempDirectory(QString &tempDirPath, QString prefix = DEFAULT_TEMP_DIR_PREFIX);
    static void removeAllTempDirectories(QString prefix = DEFAULT_TEMP_DIR_PREFIX);
    static quint64 dirSize(QString _dir_path, bool _include_subdirs = false);
    static quint32 fileCount(QString _dir_path, bool _include_subdirs = false);
    static QString resolveUnixPath(QString _file_spec);
};
}
#endif // FILE_UTILS_H
