#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QStringList>
#include "quazip.h"
#include "quazipfile.h"
#include "quazipnewinfo.h"
#include "JlCompress.h"

namespace MatisseCommon {

class FileUtils
{
public:
    static bool removeDir(const QString &dirName);
    static bool zipFiles(QString zipArchivePath, QString baseDirPath, QStringList fileNames, bool append = false);
    static bool unzipFiles(QString zipArchivePath, QDir destDir);
    static QStringList getZipFileList(QString zipArchivePath);
};
}
#endif // FILE_UTILS_H
