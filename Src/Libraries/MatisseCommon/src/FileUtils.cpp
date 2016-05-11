#include "FileUtils.h"

using namespace MatisseCommon;

bool FileUtils::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

///
/// \brief Creates or updates a zip archive with files specified.
/// Files are stored with a path relative to the base folder path specified.
///
/// \param zipArchivePath Full path to the zip file to be created or updated
/// \param baseDirPath Relative or absolute path to the base folder
/// \param fileNames A list of files expressed as relative or absolute path.
/// Files must be part of the base folder tree.
/// \param append Specifies if the archive is to be created (false) or updated (true).
/// Default value is false. In the creation mode, if the zip already exists it will be overwritten.
///
/// \return true if operation was successful (that is a zip file was created), false otherwise
bool FileUtils::zipFiles(QString zipArchivePath, QString baseDirPath, QStringList fileNames, bool append)
{
    QuaZip zip(zipArchivePath);

    if (append) {
        if (!zip.open(QuaZip::mdAppend)) {
            qCritical() << QString("Impossible to open zip file '%1' in append mode").arg(zipArchivePath);
            return false;
        }
    } else {
        if (!zip.open(QuaZip::mdCreate)) {
            qCritical() << QString("Impossible to create zip file '%1'").arg(zipArchivePath);
            return false;
        }
    }

    QuaZipFile outZipFile(&zip);

    QDir baseDir(baseDirPath);
    if (!baseDir.exists()) {
        qCritical() << QString("Base directory '%1' provided for source files is not valid. Could not archive files to zip").arg(baseDir.path());
        zip.close();
        return false;
    }

    QString baseAbsolutePath = baseDir.absolutePath();
    if (!baseAbsolutePath.endsWith("/")) {
        baseAbsolutePath.append("/");
    }

    foreach (QString fileName, fileNames) {
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            qWarning() << QString("Entry '%1' does not exist. Could not add entry to zip archive").arg(fileInfo.path());
            continue;
        }

        QString fileAbsolutePath = fileInfo.absoluteFilePath();

        if (!fileAbsolutePath.startsWith(baseAbsolutePath)) {
            qWarning() << QString("Entry '%1' is not in the scope of base path '%2'. Could not add entry to zip archive").arg(fileAbsolutePath).arg(baseAbsolutePath);
            continue;
        }

        QString fileRelativePath = fileAbsolutePath.remove(0, baseAbsolutePath.length());

        if (fileInfo.isDir()) { // ENTRY IS A FOLDER
            if (!fileRelativePath.endsWith("/")) {
                fileRelativePath.append("/");
            }

            if (!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileRelativePath, fileAbsolutePath))) {
                qWarning() << QString("Entry '%1' could not be added to zip archive").arg(fileAbsolutePath);
                continue;
            }

            outZipFile.close();
        } else {            // ENTRY IS A FILE
            QFile inFile(fileAbsolutePath);
            if (!inFile.open(QIODevice::ReadOnly)) {
                qWarning() << QString("Could not open file '%1'. File was not added to zip archive").arg(fileAbsolutePath);
                continue;
            }

            if (!outZipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(fileRelativePath, fileAbsolutePath))) {
                qWarning() << QString("Entry '%1' could not be added to zip archive").arg(fileAbsolutePath);
                inFile.close();
                continue;
            }

            qDebug() << QString("Adding entry '%1' with source path '%2' to archive...").arg(fileRelativePath).arg(fileAbsolutePath);

            /* writing file to zip archive */
            QByteArray buffer;
            int chunksize = 256;
            buffer = inFile.read(chunksize);
            while (!buffer.isEmpty()) {
                outZipFile.write(buffer);
                buffer = inFile.read(chunksize);
            }

            outZipFile.close();
            inFile.close();
        }
    }

    zip.close();

    return true;
}

bool FileUtils::unzipFiles(QString zipArchivePath, QDir destDir)
{
    QFileInfo archiveFileInfo(zipArchivePath);
    if (!archiveFileInfo.exists()) {
        qCritical() << QString("The archive '%1' does not exist. Could not unzip.").arg(zipArchivePath);
        return false;
    }

    if (!destDir.exists()) {
        qCritical() << QString("Destination directory '%1' does not exist").arg(destDir.absolutePath());
        return false;
    }

    QString destDirPath = destDir.absolutePath();
    JlCompress::extractDir(zipArchivePath, destDirPath);

    // check platform


    return true;
}

QStringList FileUtils::getZipFileList(QString zipArchivePath)
{
    QStringList fileList;

    QFileInfo archiveFileInfo(zipArchivePath);
    if (!archiveFileInfo.exists()) {
        qCritical() << QString("The archive '%1' does not exist. Could not unzip.").arg(zipArchivePath);
        return fileList; // empty list
    }

    fileList = JlCompress::getFileList(zipArchivePath);
    return fileList;
}


