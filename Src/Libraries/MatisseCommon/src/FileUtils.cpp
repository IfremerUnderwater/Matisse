#include "FileUtils.h"

namespace MatisseCommon {

/// \brief Removes a directory and all its contents recursively
///
/// \param dirName Relative or absolute path to the directory to be removed
///
/// \return true if operation was successful, false otherwise
///
bool FileUtils::removeDir(const QString& dirName) {
  bool result = true;
  QDir dir(dirName);

  if (!dir.exists()) {
    qCritical() << QString(
                       "Directory '%1' does not exist, impossible to remove")
                       .arg(dirName);
    return false;
  }

  Q_FOREACH (QFileInfo info,
             dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System |
                                   QDir::Hidden | QDir::AllDirs | QDir::Files,
                               QDir::DirsFirst)) {
    if (info.isDir()) {
      result = removeDir(info.absoluteFilePath());
    } else {
      result = QFile::remove(info.absoluteFilePath());
    }

    if (!result) {
      qCritical() << QString(
                         "Failed to remove file or directory '%1' while trying "
                         "to remove '%2' directory tree")
                         .arg(info.absoluteFilePath())
                         .arg(dirName);
      return result;
    }
  }

  result = dir.rmdir(dirName);

  if (!result) {
    qCritical()
        << QString("Impossible to delete empty directory '%1'").arg(dirName);
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
/// \param append Specifies if the archive is to be created (false) or updated
/// (true). Default value is false. In the creation mode, if the zip already
/// exists it will be overwritten.
///
/// \return true if operation was successful (that is a zip file was created),
/// false otherwise
///
bool FileUtils::zipFiles(QString zipArchivePath, QString baseDirPath,
                         QStringList fileNames, bool append) {
  QuaZip zip(zipArchivePath);

  if (append) {
    if (!zip.open(QuaZip::mdAppend)) {
      qCritical() << QString("Impossible to open zip file '%1' in append mode")
                         .arg(zipArchivePath);
      return false;
    }
  } else {
    if (!zip.open(QuaZip::mdCreate)) {
      qCritical()
          << QString("Impossible to create zip file '%1'").arg(zipArchivePath);
      return false;
    }
  }

  QuaZipFile outZipFile(&zip);

  QDir baseDir(baseDirPath);
  if (!baseDir.exists()) {
    qCritical() << QString(
                       "Base directory '%1' provided for source files is not "
                       "valid. Could not archive files to zip")
                       .arg(baseDir.path());
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
      qWarning() << QString(
                        "Entry '%1' does not exist. Could not add entry to zip "
                        "archive")
                        .arg(fileInfo.path());
      continue;
    }

    QString fileAbsolutePath = fileInfo.absoluteFilePath();

    if (!fileAbsolutePath.startsWith(baseAbsolutePath)) {
      qWarning() << QString(
                        "Entry '%1' is not in the scope of base path '%2'. "
                        "Could not add entry to zip archive")
                        .arg(fileAbsolutePath)
                        .arg(baseAbsolutePath);
      continue;
    }

    QString fileRelativePath =
        fileAbsolutePath.remove(0, baseAbsolutePath.length());

    if (fileInfo.isDir()) {  // ENTRY IS A FOLDER
      if (!fileRelativePath.endsWith("/")) {
        fileRelativePath.append("/");
      }

      if (!outZipFile.open(QIODevice::WriteOnly,
                           QuaZipNewInfo(fileRelativePath, fileAbsolutePath))) {
        qWarning() << QString("Entry '%1' could not be added to zip archive")
                          .arg(fileAbsolutePath);
        continue;
      }

      outZipFile.close();
    } else {  // ENTRY IS A FILE
      QFile inFile(fileAbsolutePath);
      if (!inFile.open(QIODevice::ReadOnly)) {
        qWarning() << QString(
                          "Could not open file '%1'. File was not added to zip "
                          "archive")
                          .arg(fileAbsolutePath);
        continue;
      }

      if (!outZipFile.open(QIODevice::WriteOnly,
                           QuaZipNewInfo(fileRelativePath, fileAbsolutePath))) {
        qWarning() << QString("Entry '%1' could not be added to zip archive")
                          .arg(fileAbsolutePath);
        inFile.close();
        continue;
      }

      qDebug() << QString(
                      "Adding entry '%1' with source path '%2' to archive...")
                      .arg(fileRelativePath)
                      .arg(fileAbsolutePath);

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

/// \brief Extracts all file entries from the zip archive file to a specfied
/// destination directory.
///
/// \param zipArchivePath Relative or absolute path to the zip archive file
/// \param destDirPath Relative or absolute path to the destination directory
///
/// \return true if the extraction was successful, false otherwise
///
bool FileUtils::unzipFiles(QString zipArchivePath, QString destDirPath) {
  QFileInfo archiveFileInfo(zipArchivePath);
  if (!archiveFileInfo.exists()) {
    qCritical() << QString("The archive '%1' does not exist. Could not unzip.")
                       .arg(zipArchivePath);
    return false;
  }

  /* check zip archive content */
  QStringList archiveFilesList = getZipEntries(zipArchivePath);
  if (archiveFilesList.isEmpty()) {
    qWarning() << QString("Archive '%1' is empty, nothing to extract")
                      .arg(zipArchivePath);
    return true;
  }

  QDir destDir(destDirPath);
  if (!destDir.exists()) {
    qCritical() << QString("Destination directory '%1' does not exist")
                       .arg(destDirPath);
    return false;
  }

  QString destDirAbsolutePath = destDir.absolutePath();
  QStringList extractedFilesList =
      JlCompress::extractDir(zipArchivePath, destDirAbsolutePath);

  if (extractedFilesList.isEmpty()) {
    qCritical() << QString("Extraction failed for the archive '%1'.")
                       .arg(zipArchivePath);
    return false;
  }

  int archiveEntriesNb = archiveFilesList.size();
  int extractedEntriesNb = extractedFilesList.size();

  if (extractedEntriesNb != archiveEntriesNb) {
    qCritical() << QString(
                       "The archive '%1' contains %2 entries, but %3 were "
                       "actually extracted")
                       .arg(zipArchivePath)
                       .arg(archiveEntriesNb)
                       .arg(extractedEntriesNb);
    return false;
  }

  return true;
}

/// \brief Retrieves the list of entries for a zip archive.
///
/// \param zipArchivePath Relative or absolute path to the zip archive file
///
/// \return the list of entries
///
QStringList FileUtils::getZipEntries(QString zipArchivePath) {
  QStringList fileList;

  QFileInfo archiveFileInfo(zipArchivePath);
  if (!archiveFileInfo.exists()) {
    qCritical() << QString("The archive '%1' does not exist. Could not unzip.")
                       .arg(zipArchivePath);
    return fileList;  // empty list
  }

  fileList = JlCompress::getFileList(zipArchivePath);
  return fileList;
}

///
/// \brief Checks wether 2 files are identical by comparing their respective MD5
/// signature.
///
/// \param filePath1 Relative or absolute path to the first file
/// \param filePath2 Relative or absolute path to the first file
///
/// \return true if files are identical, false otherwise. If filePath1 and
/// filePath2 point to the same file, the method issues a warning and returns
/// true.
///
bool FileUtils::areFilesIdentical(QString filePath1, QString filePath2) {
  QFileInfo fileInfo1(filePath1);
  QFileInfo fileInfo2(filePath2);

  if (!fileInfo1.exists()) {
    qCritical()
        << QString("The file '%1' does not exist, impossible to compare files.")
               .arg(filePath1);
    return false;
  }

  if (!fileInfo2.exists()) {
    qCritical()
        << QString("The file '%1' does not exist, impossible to compare files.")
               .arg(filePath2);
    return false;
  }

  if (fileInfo2 == fileInfo1) {
    qWarning() << QString("Trying compare file '%1' to itself").arg(filePath1);
    return true;
  }

  QFile file1(filePath1);
  QFile file2(filePath2);

  if (!file1.open(QIODevice::ReadOnly)) {
    qCritical() << QString(
                       "The file '%1' could not be opened, impossible to "
                       "compare files.")
                       .arg(filePath1);
    return false;
  }

  if (!file2.open(QIODevice::ReadOnly)) {
    qCritical() << QString(
                       "The file '%1' could not be opened, impossible to "
                       "compare files.")
                       .arg(filePath2);
    return false;
  }

  QByteArray file1Data = file1.readAll();
  QByteArray file1Hash =
      QCryptographicHash::hash(file1Data, QCryptographicHash::Md5);
  file1.close();

  QByteArray file2Data = file2.readAll();
  QByteArray file2Hash =
      QCryptographicHash::hash(file2Data, QCryptographicHash::Md5);
  file2.close();

  bool filesAreIdentical = (file2Hash == file1Hash);
  return filesAreIdentical;
}

///
/// \brief FileUtils::readPropertiesFile Parses a properties file to extract
/// key/value pairs. \param propFilePath Relative or absolute path to the
/// properties file \return the collection of key/value pairs
///
QMap<QString, QString> FileUtils::readPropertiesFile(QString propFilePath) {
  QFile propFile(propFilePath);
  QMap<QString, QString> properties;

  if (!propFile.exists()) {
    qCritical()
        << QString("The file '%1' does not exist, could not load properties")
               .arg(propFilePath);
    return properties;
  }

  if (!propFile.open(QIODevice::ReadOnly)) {
    qCritical()
        << QString(
               "I/O error while opening file '%1', could not load properties")
               .arg(propFilePath);
    return properties;
  }

  QString resolvedPropDefPattern =
      QString(PROPERTY_DEFINITION_PATTERN).arg(PROPERTY_NAME_PATTERN);
  QRegExp propDefRex(resolvedPropDefPattern);

  QTextStream in(&propFile);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();

    if (line.isEmpty()) {
      continue;
    }

    if (line.startsWith('#')) {
      // comment
      continue;
    }

    if (!propDefRex.exactMatch(line)) {
      qWarning() << QString(
                        "Line does not match property definition pattern and "
                        "will be ignored :\n%1")
                        .arg(line);
      continue;
    }

    QStringList parts = line.split("=");

    if (parts.size() < 2) {
      qCritical() << QString("Line could not be parsed :\n%1").arg(line);
      continue;
    }

    QString key = parts.at(0);
    QString value = parts.at(1);

    if (properties.contains(key)) {
      qWarning() << QString(
                        "Property '%1' defined more than once, only first "
                        "value will be kept.")
                        .arg(key);
      continue;
    }

    properties.insert(key, value);
  }

  propFile.close();

  return properties;
}

bool FileUtils::createTempDirectory(QString& tempDirPath, QString prefix) {
  qDebug() << "Creating new temp directories...";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  QString tempRootLocation =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  QString tempRootLocation =
      QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#endif
  QString timestampFormat = QString(TEMP_DIR_TEMPLATE_FORMAT).arg(prefix);

  bool done = false;
  int trials = 0;

  while (!done) {
    if (trials == TEMP_DIR_MAX_TRIALS) {
      break;
    }

    QDateTime now = QDateTime::currentDateTime();
    QString timestamp = now.toString(timestampFormat);
    QString tempDirAttemptPath =
        tempRootLocation + QDir::separator() + timestamp;

    QDir tempDir(tempDirAttemptPath);
    if (tempDir.exists()) {
      qWarning() << QString("The temp directory '%1' already exists")
                        .arg(tempDirAttemptPath);
      QThread::currentThread()->wait(
          2);  // wait 2ms to ensure that next timestamp is different
      trials++;
      continue;
    }

    bool created = tempDir.mkpath(".");
    if (!created) {
      qWarning() << QString("Could not create temp directory '%1'")
                        .arg(tempDirAttemptPath);
      trials++;
      continue;
    }

    tempDirPath = tempDirAttemptPath;
    done = true;
  }

  if (done) {
    return true;
  } else {
    qCritical() << QString("Failed to create temp directory");
    return false;
  }
}

void FileUtils::removeAllTempDirectories(QString prefix) {
  qDebug() << "Removing all temp directories...";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
  QString tempRootLocation =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  QString tempRootLocation =
      QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#endif
  QDir tempRootDir(tempRootLocation);

  QString nameFilter = prefix + "*";
  QStringList filters;
  filters << nameFilter;

  QStringList entries = tempRootDir.entryList(filters, QDir::Dirs, QDir::Name);

  foreach (QString entry, entries) {
    QString tempDirPath = tempRootLocation + "/" + entry;
    bool removed = removeDir(tempDirPath);
    if (!removed) {
      qWarning()
          << QString("Could not remove temp directory '%1'").arg(tempDirPath);
    }
  }
}

quint64 MatisseCommon::FileUtils::dirSize(QString _dir_path,
                                             bool _include_subdirs) {
  
  quint64 total_size = 0;

  QDir dir(_dir_path);

  if (!dir.exists()) {
    qWarning() << QString(
                      "FileUtils::dirSize: cannot compute size for dir '%1' "
                      ": dir does not exist")
                      .arg(_dir_path);  
    return 0;
  }

  /* Compute size at directory level */
  QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
  for (QFileInfo file : files) {
    total_size += file.size();
  }

  if (_include_subdirs) { // compute recursively
    QFileInfoList subdirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (QFileInfo subdir : subdirs) {
      QString subdir_path = subdir.absoluteFilePath();
      total_size += dirSize(subdir_path, true);
    }
  }
  
  return total_size;
}

quint32 FileUtils::fileCount(QString _dir_path, bool _include_subdirs) {

  quint32 total_count = 0;

  QDir dir(_dir_path);

  if (!dir.exists()) {
    qWarning() << QString(
                      "FileUtils::fileCount: cannot count files for dir '%1' "
                      ": dir does not exist")
                      .arg(_dir_path);
    return 0;
  }

  /* Count files at directory level */
  QFileInfoList files = dir.entryInfoList(QDir::Files | QDir::NoSymLinks);
  total_count += files.size();

  if (_include_subdirs) {  // count recursively
    QFileInfoList subdirs =
        dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (QFileInfo subdir : subdirs) {
      QString subdir_path = subdir.absoluteFilePath();
      total_count += fileCount(subdir_path, true);
    }
  }

  return total_count;
}

QString FileUtils::resolveUnixPath(QString _file_spec) {
  
  /* Regular paths are left unchanged */
  if (!_file_spec.startsWith("~")) {
    return _file_spec;
  }

  /* Current user : substituting ~ by home path */
  if (_file_spec.startsWith("~/")) {
    QString initial_spec = _file_spec;
    QString resolved_path = _file_spec.replace(0, 1, QDir::homePath());
    qDebug() << QString("FileUtils: resolving UNIX path '%1' -> '%2'")
                    .arg(initial_spec, resolved_path);
    return resolved_path;
  }

  /* Path addressing other users, type '~johnny/...' : not supported */
  qWarning() << QString("FileUtils: UNIX path '%1' addresses a specific user. Not supported, path is left unchanged").arg(_file_spec);
  return _file_spec;
}

}  // namespace MatisseCommon