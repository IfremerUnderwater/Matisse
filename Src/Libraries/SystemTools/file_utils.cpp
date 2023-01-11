#include "file_utils.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace system_tools {

bool FileUtils::copyDir(QString _src_dir_path, QString _dst_dir_path, bool _recurse, bool _overwrite) {

    if (_recurse) {
        qCritical() << "FileUtils::copyDir: recurse dir copy not supported yet, copy aborted";
        return false;
    }

    QDir src_dir(_src_dir_path);
    if (!src_dir.exists()) {
        qCritical() << QString("FileUtils::copyDir: source dir '%1' does not exist, copy aborted").arg(_src_dir_path);
        return false;
    }

    QDir dst_dir(_dst_dir_path);
    if (!dst_dir.exists()) {
        qCritical() << QString("FileUtils::copyDir: destination dir '%1' does not exist, copy aborted").arg(_dst_dir_path);
        return false;
    }

    QStringList dir_entries;

    dir_entries = src_dir.entryList((QDir::Files));

    foreach (QString entry, dir_entries) {
        QString entry_path = _src_dir_path + QDir::separator() + entry;
        QFileInfo src_finfo(entry_path);

        if (src_finfo.isSymLink()) {
            continue;
        }

        QString dst_file_path = _dst_dir_path + QDir::separator() + entry;
        QFileInfo dst_finfo(dst_file_path);

        if (dst_finfo.exists()) {
            if (!_overwrite) {
                continue;
            }

            QFile dst_file_to_remove(dst_file_path);
            bool removed = dst_file_to_remove.remove();
            if (!removed) {
                qWarning() << QString("FileUtils::copyDir: file '%1' exists in destination and could not be removed, skipping file copy")
                              .arg(entry_path);
                continue;
            }
        }

        QFile src_file(entry_path);
        bool copied = src_file.copy(dst_file_path);

        if (!copied) {
            qWarning() << QString("FileUtils::copyDir: could not copy file '%1' to dir '%2'").arg(entry_path).arg(_dst_dir_path);
        }
    }

    return true;
}


/// \brief Removes a directory and all its contents recursively
///
/// \param _dir_name Relative or absolute path to the directory to be removed
///
/// \return true if operation was successful, false otherwise
///
bool FileUtils::removeDir(const QString& _dir_name) {
    bool result = true;
    QDir dir(_dir_name);

    if (!dir.exists()) {
        qCritical() << QString(
                           "Directory '%1' does not exist, impossible to remove")
                       .arg(_dir_name);
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
                           .arg(_dir_name);
            return result;
        }
    }

    result = dir.rmdir(_dir_name);

    if (!result) {
        qCritical()
                << QString("Impossible to delete empty directory '%1'").arg(_dir_name);
    }

    return result;
}

///
/// \brief Creates or updates a zip archive with files specified.
/// Files are stored with a path relative to the base folder path specified.
///
/// \param _zip_archive_path Full path to the zip file to be created or updated
/// \param _base_dir_path Relative or absolute path to the base folder
/// \param _file_names A list of files expressed as relative or absolute path.
/// Files must be part of the base folder tree.
/// \param _append Specifies if the archive is to be created (false) or updated
/// (true). Default value is false. In the creation mode, if the zip already
/// exists it will be overwritten.
///
/// \return true if operation was successful (that is a zip file was created),
/// false otherwise
///
bool FileUtils::zipFiles(QString _zip_archive_path, QString _base_dir_path,
                         QStringList _file_names, bool _append) {
    QuaZip zip(_zip_archive_path);

    if (_append) {
        if (!zip.open(QuaZip::mdAppend)) {
            qCritical() << QString("Impossible to open zip file '%1' in append mode")
                           .arg(_zip_archive_path);
            return false;
        }
    } else {
        if (!zip.open(QuaZip::mdCreate)) {
            qCritical()
                    << QString("Impossible to create zip file '%1'").arg(_zip_archive_path);
            return false;
        }
    }

    QuaZipFile out_zip_file(&zip);

    QDir base_dir(_base_dir_path);
    if (!base_dir.exists()) {
        qCritical() << QString(
                           "Base directory '%1' provided for source files is not "
                           "valid. Could not archive files to zip")
                       .arg(base_dir.path());
        zip.close();
        return false;
    }

    QString base_absolute_path = base_dir.absolutePath();
    if (!base_absolute_path.endsWith("/")) {
        base_absolute_path.append("/");
    }

    foreach (QString file_name, _file_names) {
        QFileInfo fileInfo(file_name);
        if (!fileInfo.exists()) {
            qWarning() << QString(
                              "Entry '%1' does not exist. Could not add entry to zip "
                              "archive")
                          .arg(fileInfo.path());
            continue;
        }

        QString file_absolute_path = fileInfo.absoluteFilePath();

        if (!file_absolute_path.startsWith(base_absolute_path)) {
            qWarning() << QString(
                              "Entry '%1' is not in the scope of base path '%2'. "
                              "Could not add entry to zip archive")
                          .arg(file_absolute_path)
                          .arg(base_absolute_path);
            continue;
        }

        QString file_relative_path =
                file_absolute_path.remove(0, base_absolute_path.length());

        if (fileInfo.isDir()) {  // ENTRY IS A FOLDER
            if (!file_relative_path.endsWith("/")) {
                file_relative_path.append("/");
            }

            if (!out_zip_file.open(QIODevice::WriteOnly,
                                   QuaZipNewInfo(file_relative_path, file_absolute_path))) {
                qWarning() << QString("Entry '%1' could not be added to zip archive")
                              .arg(file_absolute_path);
                continue;
            }

            out_zip_file.close();
        } else {  // ENTRY IS A FILE
            QFile in_file(file_absolute_path);
            if (!in_file.open(QIODevice::ReadOnly)) {
                qWarning() << QString(
                                  "Could not open file '%1'. File was not added to zip "
                                  "archive")
                              .arg(file_absolute_path);
                continue;
            }

            if (!out_zip_file.open(QIODevice::WriteOnly,
                                   QuaZipNewInfo(file_relative_path, file_absolute_path))) {
                qWarning() << QString("Entry '%1' could not be added to zip archive")
                              .arg(file_absolute_path);
                in_file.close();
                continue;
            }

            qDebug() << QString(
                            "Adding entry '%1' with source path '%2' to archive...")
                        .arg(file_relative_path)
                        .arg(file_absolute_path);

            /* writing file to zip archive */
            QByteArray buffer;
            int chunksize = 256;
            buffer = in_file.read(chunksize);
            while (!buffer.isEmpty()) {
                out_zip_file.write(buffer);
                buffer = in_file.read(chunksize);
            }

            out_zip_file.close();
            in_file.close();
        }
    }

    zip.close();

    return true;
}

/// \brief Extracts all file entries from the zip archive file to a specfied
/// destination directory.
///
/// \param _zip_archive_path Relative or absolute path to the zip archive file
/// \param _dest_dir_path Relative or absolute path to the destination directory
///
/// \return true if the extraction was successful, false otherwise
///
bool FileUtils::unzipFiles(QString _zip_archive_path, QString _dest_dir_path) {
    QFileInfo archive_file_info(_zip_archive_path);
    if (!archive_file_info.exists()) {
        qCritical() << QString("The archive '%1' does not exist. Could not unzip.")
                       .arg(_zip_archive_path);
        return false;
    }

    /* check zip archive content */
    QStringList archive_files_list = getZipEntries(_zip_archive_path);
    if (archive_files_list.isEmpty()) {
        qWarning() << QString("Archive '%1' is empty, nothing to extract")
                      .arg(_zip_archive_path);
        return true;
    }

    QDir dest_dir(_dest_dir_path);
    if (!dest_dir.exists()) {
        qCritical() << QString("Destination directory '%1' does not exist")
                       .arg(_dest_dir_path);
        return false;
    }

    QString dest_dir_absolute_path = dest_dir.absolutePath();
    QStringList extracted_files_list =
            JlCompress::extractDir(_zip_archive_path, dest_dir_absolute_path);

    if (extracted_files_list.isEmpty()) {
        qCritical() << QString("Extraction failed for the archive '%1'.")
                       .arg(_zip_archive_path);
        return false;
    }

    int archive_entries_nb = archive_files_list.size();
    int extracted_entries_nb = extracted_files_list.size();

    if (extracted_entries_nb != archive_entries_nb) {
        qCritical() << QString(
                           "The archive '%1' contains %2 entries, but %3 were "
                           "actually extracted")
                       .arg(_zip_archive_path)
                       .arg(archive_entries_nb)
                       .arg(extracted_entries_nb);
        return false;
    }

    return true;
}

/// \brief Retrieves the list of entries for a zip archive.
///
/// \param _zip_archive_path Relative or absolute path to the zip archive file
///
/// \return the list of entries
///
QStringList FileUtils::getZipEntries(QString _zip_archive_path) {
    QStringList file_list;

    QFileInfo archive_file_info(_zip_archive_path);
    if (!archive_file_info.exists()) {
        qCritical() << QString("The archive '%1' does not exist. Could not unzip.")
                       .arg(_zip_archive_path);
        return file_list;  // empty list
    }

    file_list = JlCompress::getFileList(_zip_archive_path);
    return file_list;
}

///
/// \brief Checks wether 2 files are identical by comparing their respective MD5
/// signature.
///
/// \param _file_path1 Relative or absolute path to the first file
/// \param _file_path2 Relative or absolute path to the first file
///
/// \return true if files are identical, false otherwise. If _file_path1 and
/// _file_path2 point to the same file, the method issues a warning and returns
/// true.
///
bool FileUtils::areFilesIdentical(QString _file_path1, QString _file_path2) {
    QFileInfo file_info1(_file_path1);
    QFileInfo file_info2(_file_path2);

    if (!file_info1.exists()) {
        qCritical()
                << QString("The file '%1' does not exist, impossible to compare files.")
                   .arg(_file_path1);
        return false;
    }

    if (!file_info2.exists()) {
        qCritical()
                << QString("The file '%1' does not exist, impossible to compare files.")
                   .arg(_file_path2);
        return false;
    }

    if (file_info2 == file_info1) {
        qWarning() << QString("Trying compare file '%1' to itself").arg(_file_path1);
        return true;
    }

    QFile file1(_file_path1);
    QFile file2(_file_path2);

    if (!file1.open(QIODevice::ReadOnly)) {
        qCritical() << QString(
                           "The file '%1' could not be opened, impossible to "
                           "compare files.")
                       .arg(_file_path1);
        return false;
    }

    if (!file2.open(QIODevice::ReadOnly)) {
        qCritical() << QString(
                           "The file '%1' could not be opened, impossible to "
                           "compare files.")
                       .arg(_file_path2);
        return false;
    }

    QByteArray file1_data = file1.readAll();
    QByteArray file1_hash =
            QCryptographicHash::hash(file1_data, QCryptographicHash::Md5);
    file1.close();

    QByteArray file2_data = file2.readAll();
    QByteArray file2_hash =
            QCryptographicHash::hash(file2_data, QCryptographicHash::Md5);
    file2.close();

    bool files_are_identical = (file2_hash == file1_hash);
    return files_are_identical;
}

///
/// \brief FileUtils::readPropertiesFile Parses a properties file to extract
/// key/value pairs. \param _prop_file_path Relative or absolute path to the
/// properties file \return the collection of key/value pairs
///
QMap<QString, QString> FileUtils::readPropertiesFile(QString _prop_file_path) {
    QFile prop_file(_prop_file_path);
    QMap<QString, QString> properties;

    if (!prop_file.exists()) {
        qCritical()
                << QString("The file '%1' does not exist, could not load properties")
                   .arg(_prop_file_path);
        return properties;
    }

    if (!prop_file.open(QIODevice::ReadOnly)) {
        qCritical()
                << QString(
                       "I/O error while opening file '%1', could not load properties")
                   .arg(_prop_file_path);
        return properties;
    }

    QString resolved_prop_def_pattern =
            QString(PROPERTY_DEFINITION_PATTERN).arg(PROPERTY_NAME_PATTERN);
    QRegExp prop_def_rex(resolved_prop_def_pattern);

    QTextStream in(&prop_file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) {
            continue;
        }

        if (line.startsWith('#')) {
            // comment
            continue;
        }

        if (!prop_def_rex.exactMatch(line)) {
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

    prop_file.close();

    return properties;
}

bool FileUtils::createTempDirectory(QString& _temp_dir_path, QString _prefix) {
    qDebug() << "Creating new temp directories...";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QString temp_root_location =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
    QString tempRootLocation =
            QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#endif
    QString timestamp_format = QString(TEMP_DIR_TEMPLATE_FORMAT).arg(_prefix);

    bool done = false;
    int trials = 0;

    while (!done) {
        if (trials == TEMP_DIR_MAX_TRIALS) {
            break;
        }

        QDateTime now = QDateTime::currentDateTime();
        QString timestamp = now.toString(timestamp_format);
        QString temp_dir_attempt_path =
                temp_root_location + QDir::separator() + timestamp;

        QDir temp_dir(temp_dir_attempt_path);
        if (temp_dir.exists()) {
            qWarning() << QString("The temp directory '%1' already exists")
                          .arg(temp_dir_attempt_path);
            QThread::currentThread()->wait(
                        2);  // wait 2ms to ensure that next timestamp is different
            trials++;
            continue;
        }

        bool created = temp_dir.mkpath(".");
        if (!created) {
            qWarning() << QString("Could not create temp directory '%1'")
                          .arg(temp_dir_attempt_path);
            trials++;
            continue;
        }

        _temp_dir_path = temp_dir_attempt_path;
        done = true;
    }

    if (done) {
        return true;
    } else {
        qCritical() << QString("Failed to create temp directory");
        return false;
    }
}

void FileUtils::removeAllTempDirectories(QString _prefix) {
    qDebug() << "Removing all temp directories...";

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QString temp_root_location =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
    QString tempRootLocation =
            QDesktopServices::storageLocation(QDesktopServices::TempLocation);
#endif
    QDir temp_root_dir(temp_root_location);

    QString name_filter = _prefix + "*";
    QStringList filters;
    filters << name_filter;

    QStringList entries = temp_root_dir.entryList(filters, QDir::Dirs, QDir::Name);

    foreach (QString entry, entries) {
        QString temp_dir_path = temp_root_location + "/" + entry;
        bool removed = removeDir(temp_dir_path);
        if (!removed) {
            qWarning()
                    << QString("Could not remove temp directory '%1'").arg(temp_dir_path);
        }
    }
}

quint64 FileUtils::dirSize(QString _dir_path,
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

QString FileUtils::getParentDir(QString _current_dir) {
    if (!isValidUnixPath(_current_dir)) {
        qCritical() << QString("FileUtils: dir '%1' is not a valid path").arg(_current_dir);
        return _current_dir;
    }

    /* Looking for the parent dir of a relative path is not supported */
    if (!isValidUnixPath(_current_dir, true)) {
        qWarning() << QString("FileUtils: dir '%1' is not an absolute path, cannot resolve parent path").arg(_current_dir);
        return _current_dir;
    }

    if (isRootPath(_current_dir)) {
        qWarning() << QString("FileUtils: dir '%1' is root dir, cannot get parent dir").arg(_current_dir);
        return _current_dir;
    }

    /* Find path leaf */
    QRegularExpression path_exp(ABSOLUTE_UNIX_PATH_PATTERN);
    QRegularExpressionMatch path_match = path_exp.match(_current_dir);
    int last = path_match.lastCapturedIndex();
    int pos = path_match.capturedStart(last);

    // qDebug() << QString("FileUtils: path leaf is '%1'").arg(path_match.capturedView(last));

    /* Remove leaf from path to get parent path */
    QString parent_path = _current_dir.left(pos);
    if (parent_path.isEmpty()) {
        /* Reached the root */
        parent_path = "/";
    }

    return parent_path;
}

bool FileUtils::isValidUnixPath(QString _path, bool _exclude_relative_path)
{
    QString unix_path_pattern = (_exclude_relative_path) ? ABSOLUTE_UNIX_PATH_PATTERN : GENERIC_UNIX_PATH_PATTERN;

    QRegularExpression unix_exp(unix_path_pattern);
    QRegularExpressionMatch unix_match = unix_exp.match(_path);

//    qDebug() << QString("FileUtils: Pattern is %1").arg(unix_exp.pattern());

    bool is_unix_path = unix_match.hasMatch();

    if (is_unix_path) {
        qDebug() << QString("FileUtils: Path '%1' is valid UNIX path").arg(_path);
    }

    return is_unix_path;
}

bool FileUtils::isRootPath(QString _path)
{
    QString root_dir_pattern = UNIX_ROOT_PATH_PATTERN;
    QRegularExpression root_exp(root_dir_pattern);
    QRegularExpressionMatch root_match = root_exp.match(_path);

    return root_match.hasMatch();
}


}  // namespace system_tools
