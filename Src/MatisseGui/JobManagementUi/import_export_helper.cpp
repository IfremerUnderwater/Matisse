#include <QMessageBox>

#include "import_export_helper.h"

namespace matisse {

const QString ImportExportHelper::ASSEMBLY_EXPORT_PREFIX = QString("assembly_export_");
const QString ImportExportHelper::JOB_EXPORT_PREFIX = QString("job_export_");

ImportExportHelper::ImportExportHelper(QObject *_parent) :
    QObject(_parent)
{
    m_parent_widget = static_cast<QWidget*>(_parent);
}

void ImportExportHelper::init()
{
    checkImportExportDirs();
}

void ImportExportHelper::executeExportWorkflow(bool _is_job_export_action, QString _entity_name, bool _is_custom_export,
                                               QString _custom_prefix, QString _custom_export_path) {

    // reinit custom export bundle name
    m_custom_export_filename = "";

    /* common translated labels */
    QString export_failed_title = tr("Export failure");
    QString export_title = (_is_job_export_action) ? tr("Taks export") : tr("Export processing chain");
    QString success_mssage = (_is_job_export_action) ? tr("Task '%1' exported in '%2'") : tr("Processing chain '%1' exported in '%2'");
    QString operation_failed_message = tr("Operation failed.");

    AssemblyDefinition *assembly = NULL;
    JobDefinition *job = NULL;

    QString entity_filename = "";
    QString entity_prefix = "";

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    if (_is_job_export_action) {
        job = process_data_manager->getJob(_entity_name);
        if (!job) {
            // unconsistent case, no user message
            qCritical() << QString("Job '%1' not found in repository. Job could not be exported.").arg(_entity_name);
            return;
        }

        entity_filename = job->filename();
        entity_prefix = (_is_custom_export) ? _custom_prefix : JOB_EXPORT_PREFIX;

        qDebug() << QString("Exporting job '%1'...").arg(_entity_name);

    } else {
        assembly = process_data_manager->getAssembly(_entity_name);
        if (!assembly) {
            // unconsistent case, no user message
            qCritical() << QString("Assembly '%1' not found in repository. Assembly could not be exported.").arg(_entity_name);
            return;
        }

        entity_filename = assembly->filename();
        entity_prefix = ASSEMBLY_EXPORT_PREFIX;

        qDebug() << QString("Exporting assembly '%1'...").arg(_entity_name);
    }


    QString export_path_root = (_is_custom_export) ? _custom_export_path : m_export_path;

    QString normalized_entity_name = entity_filename;
    normalized_entity_name.chop(4); // remove ".xml" suffix
    QString export_filename = export_path_root + QDir::separator() + entity_prefix + normalized_entity_name + ".zip";

    QStringList file_names;

    QString assembly_name = "";

    if (_is_job_export_action) {
        QFileInfo job_file(process_data_manager->getJobsBasePath() + QDir::separator() + entity_filename);
        if (!job_file.exists()) {
            qCritical() << QString("File not found for job '%1'. Job could not be exported.").arg(_entity_name);
            QMessageBox::critical(m_parent_widget, export_failed_title, tr("Task file '%1' does not exist.\n").arg(job_file.absoluteFilePath()).append(operation_failed_message));
            return;
        }

        file_names << job_file.absoluteFilePath();

        QFileInfo job_parameters_file(process_data_manager->getJobParametersFilePath(_entity_name));
        if (!job_parameters_file.exists()) {
            qCritical() << QString("Parameters file not found for job '%1'. Job could not be exported.").arg(_entity_name);
            QMessageBox::critical(m_parent_widget, export_failed_title, tr("Task parameter file '%1' does not exist.\n").arg(job_parameters_file.absoluteFilePath())
                                  .append(operation_failed_message));
            return;
        }

        file_names << job_parameters_file.absoluteFilePath();

        assembly_name = job->assemblyName();

        assembly = process_data_manager->getAssembly(assembly_name);
        if (!assembly) {
            qCritical() << QString("Assembly '%1' could not be found in local repository").arg(assembly_name);
            QMessageBox::critical(m_parent_widget, export_failed_title, tr("Cannot load assembly '%1' parent from task '%2'.\n").arg(assembly_name).arg(_entity_name)
                                  .append(operation_failed_message));
            return;
        }
    } else {
        assembly_name = _entity_name;
    }

    QString assembly_filename = assembly->filename();

    QFileInfo assembly_file(process_data_manager->getAssembliesPath() + QDir::separator() + assembly_filename);
    if (!assembly_file.exists()) {
        qCritical() << QString("File not found for assembly '%1'. Assembly/Job could not be exported.").arg(assembly_name);
        QMessageBox::critical(m_parent_widget, export_failed_title, tr("Assembly file' '%1' does not exist.\n").arg(assembly_file.absoluteFilePath())
                              .append(operation_failed_message));
        return;
    }

    file_names << assembly_file.absoluteFilePath();

    QFileInfo assembly_parameters_file(process_data_manager->getAssembliesParametersPath() + QDir::separator() + assembly_filename);
    if (assembly_parameters_file.exists()) {
        file_names << assembly_parameters_file.absoluteFilePath();
    } else {
        qWarning() << QString("Parameters file not found for assembly '%1'. Assembly/Job will be exported without a parameters template file.").arg(assembly_name);
    }

    SystemDataManager* system_data_manager = SystemDataManager::instance();

    system_data_manager->writePlatformSummary();
    system_data_manager->writePlatformEnvDump();

    file_names << system_data_manager->getPlatformSummaryFilePath();
    file_names << system_data_manager->getPlatformEnvDumpFilePath();

    FileUtils::zipFiles(export_filename, system_data_manager->getDataRootDir(), file_names);

    if (_is_custom_export) {
        /* Store path for later use */
        m_custom_export_filename = export_filename;
    } else {
        /* Display export confirmation */
        QMessageBox::information(
                    m_parent_widget,
                    export_title,
                    success_mssage.arg(_entity_name).arg(QDir(m_export_path).absolutePath()));
    }
}

bool ImportExportHelper::executeImportWorkflow(bool _is_job_import_action, QString _current_assembly_name)
{
    /* PROMPT USER FOR FILE TO IMPORT */
    QString file_dialog_title = (_is_job_import_action) ? tr("Import a task...") : tr("Import assembly...");
    QString import_file_path = QFileDialog::getOpenFileName(m_parent_widget, file_dialog_title, m_import_path, tr("Export file (*.zip)"));

    if (import_file_path.isEmpty()) {
        // cancelling import operation
        return false;
    }

    /* CHECKING SELECTED IMPORT FILE */
    qDebug() << "Checking assembly/job import file...";

    SystemDataManager* system_data_manager = SystemDataManager::instance();
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString data_root_dir = system_data_manager->getDataRootDir();
    QDir root(data_root_dir);

    QString user_data_path = system_data_manager->getUserDataPath();
    QString user_data_relative_path = root.relativeFilePath(user_data_path);

    QString assembly_file_pattern = process_data_manager->getAssemblyFilePattern();
    QString assembly_parameters_file_pattern = process_data_manager->getAssemblyParametersFilePattern();

    QRegExp assembly_rex(assembly_file_pattern);
    QRegExp assembly_parameters_rex(assembly_parameters_file_pattern);

    QString job_file_pattern = process_data_manager->getJobFilePattern();
    QString job_parameters_file_pattern = process_data_manager->getJobParametersFilePattern();

    QRegExp job_rex(job_file_pattern);
    QRegExp job_parameters_rex(job_parameters_file_pattern);

    QString remote_assembly_file_path = "";
    QString remote_assembly_parameters_file_path = "";

    QString remote_job_file = "";
    QString remote_job_parameters_file = "";

    QString platform_file_path = system_data_manager->getPlatformSummaryFilePath();
    QString remote_platform_file = root.relativeFilePath(platform_file_path);

    bool found_assembly_file = false;
    bool found_assembly_parameters_file = false;
    bool found_job_file = false;
    bool found_job_parameters_file = false;
    bool found_platform_file = false;

    QString import_failed_title = tr("Import failed");
    QString import_title = (_is_job_import_action) ? tr("Task import") : tr("Processing chain import");
    QString are_you_sure_continue_message = tr("Sure to continue ?");
    QString are_you_sure_replace_message = tr("Sure you want to erase ?");
    QString operation_failed_message = tr("Operation failed.");
    QString operation_temporarily_failed_message = tr("Operation temporary failed.\n");
    QString try_again_later_message = tr("Try again later.");
    QString remove_file_manually = tr("Delete manually or try again later.");

    // Parsing archive files to verify that all expected files are present
    QStringList files_list = FileUtils::getZipEntries(import_file_path);

    foreach (QString file, files_list) {
        if (file.startsWith(user_data_relative_path)) {
            if (assembly_rex.exactMatch(file)) {
                qDebug() << "Found assembly file " << file;
                remote_assembly_file_path = file;
                found_assembly_file = true;
            } else if (assembly_parameters_rex.exactMatch(file)) {
                qDebug() << "Found assembly parameters file " << file;
                remote_assembly_parameters_file_path = file;
                found_assembly_parameters_file = true;
            }

            if (_is_job_import_action) {
                if (job_rex.exactMatch(file)) {
                    qDebug() << "Found job file " << file;
                    remote_job_file = file;
                    found_job_file = true;
                } else if (job_parameters_rex.exactMatch(file)) {
                    qDebug() << "Found job parameters file " << file;
                    remote_job_parameters_file = file;
                    found_job_parameters_file = true;
                }
            }
        } else {
            if (file == remote_platform_file) {
                qDebug() << "Found platform file " << remote_platform_file;
                found_platform_file = true;
            }
        }
    }

    if (_is_job_import_action) {
        if (!found_job_file) {
            qCritical() << "Job file could not be found in the export archive " + import_file_path;
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Job file could not be found in the export archive."));
            return false;
        }

        if (!found_job_parameters_file) {
            qCritical() << "Job parameters file could not be found in the export archive " + import_file_path;
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Job parameters file could not be found in the export archive."));
            return false;
        }
    }

    if (!found_assembly_file) {
        qCritical() << "Assembly file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("Assembly file could not be found in the export archive."));
        return false;
    }

    if (!found_assembly_parameters_file) {
        qCritical() << "Assembly parameters file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("Assembly parameters file could not be found in the export archive ."));
        return false;
    }

    if (!found_platform_file) {
        qCritical() << "Remote platform summary file could not be found in the export archive " + import_file_path;
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("Remote platform summary file could not be found in the export archive."));
        return false;
    }

    /* EXTRACTING ASSEMBLY EXPORT FILE */
    qDebug() << QString("Extracting assembly/job export file '%1'").arg(import_file_path);

    QString temp_import_dir_path;
    bool temp_dir_created = FileUtils::createTempDirectory(temp_import_dir_path);

    if (!temp_dir_created) {
        qCritical() << QString("Could not create temporary importation dir '%1'.").arg(temp_import_dir_path);
        QMessageBox::critical(m_parent_widget, import_failed_title, operation_temporarily_failed_message.append(try_again_later_message));
        return false;
    }

    bool unzipped = FileUtils::unzipFiles(import_file_path, temp_import_dir_path);

    if (!unzipped) {
        qCritical() << QString("Could not unzip file '%1' to temporary importation dir '%2'.").arg(import_file_path).arg(temp_import_dir_path);
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not unzip file '%1'.\n")
                              .arg(import_file_path).append(try_again_later_message));
        return false;
    }

    /* COMPARING REMOTE AND LOCAL PLATFORM */
    QString temp_remote_platform_file = temp_import_dir_path + QDir::separator() + remote_platform_file;

    bool read = system_data_manager->readRemotePlatformSummary(temp_remote_platform_file);
    if (!read) {
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("Import file invalid.\n").append(operation_failed_message));
        return false;
    }

    PlatformComparisonStatus *comparison_status = system_data_manager->compareRemoteAndLocalPlatform();

    if (!comparison_status) {
        QMessageBox::critical(m_parent_widget, import_failed_title, tr("No information available on distant platform.\n").append(operation_failed_message));
        return false;
    }


    if (comparison_status->doesMatch()) {
        qDebug() << "Remote and local platforms are identical.";
    } else {

        /* Comparing remote and local Matisse versions */

        if (comparison_status->matisseCompared().m_version_compare == DIFFERENT) {
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Distant and local Matisse version couldn't be compared.\n").append(operation_failed_message));
            return false;
        } else if (comparison_status->matisseCompared().m_version_compare == NEWER) {
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Archive comes from newer Matisse version than this one.\n").append(operation_failed_message));
            return false;
        } else if (comparison_status->matisseCompared().m_version_compare == OLDER) {
            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("You are trying to import from an older version of Matisse. Sure to continue ?\n").append(are_you_sure_continue_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }
        }
        /* If Matisse versions are identical, execution continues */

        QStringList platform_gap_messages = comparison_status->getPlatformGapMessages();
        if (!platform_gap_messages.isEmpty()) {

            // build stack for platform gap messages
            QString platform_gap_messages_stack;

            for (int i = 0; i < platform_gap_messages.size() ; i++) {
                QString message = platform_gap_messages.at(i);
                platform_gap_messages_stack.append(i).append(": ").append(message);
                if (i < platform_gap_messages.size() - 1) {
                    platform_gap_messages_stack.append("\n");
                }
            }

            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("You are trying to import a file from a different system version (see below).\n").append(are_you_sure_continue_message)
                        .append("\n").append(platform_gap_messages_stack),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }
        }

    }

    /* CHECKING JOB / ASSEMBLY CONSISTENCY */

    QString source_job_file_path = "";
    QString job_name = "";
    QString assembly_name;

    if (_is_job_import_action) {
        qDebug() << "Checking if imported job is consistent with selected assembly...";

        QString temp_user_data_path = temp_import_dir_path + "/" + user_data_relative_path;

        // instantiate new data manager, not forgetting to delete it after use
        ProcessDataManager* temp_data_mgr = ProcessDataManager::newTempInstance();
        temp_data_mgr->init(temp_import_dir_path, temp_user_data_path);

        source_job_file_path = temp_import_dir_path + "/" + remote_job_file;
        QFileInfo temp_job_file_info(source_job_file_path);
        if (!temp_job_file_info.exists()) {
            // Technically unconsistent case : something went wront when unzipping
            qCritical() << "Temp job file not found, archive unzipping failed";
            ProcessDataManager::deleteTempInstance();
            return false;
        }

        QString temp_job_file_name = temp_job_file_info.fileName();
        bool read = temp_data_mgr->readJobFile(temp_job_file_name);

        if (!read) {
            qCritical() << QString("The job file contained in export archive is invalid.");
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Task file in archive is invalid.\n")
                                  .append(operation_failed_message));
            ProcessDataManager::deleteTempInstance();
            return false;
        }

        QStringList temp_job_names = temp_data_mgr->getJobsNames();
        if (temp_job_names.isEmpty()) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            ProcessDataManager::deleteTempInstance();
            return false;
        }

        job_name = temp_job_names.at(0);
        JobDefinition *job = temp_data_mgr->getJob(job_name);

        ProcessDataManager::deleteTempInstance();

        if (!job) {
            // technically unconsistent case
            qCritical() << "Source job file was read from export archive but was not handled correctly by the data manager";
            return false;
        }

        assembly_name = job->assemblyName();

        if (assembly_name != _current_assembly_name) {
            qCritical() << QString("The job file contained in export archive is invalid.").arg(source_job_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("The task you're trying to import does not match the selected assembly.\n")
                                  .append(operation_failed_message));
            return false;
        }
    }


    /* CHECKING IF JOB/ASSEMBLY FILES ARE ALREADY PRESENT ON LOCAL PLATFORM */

    QString target_job_file_path = "";
    QString target_job_parameters_file_path = "";

    QString target_assembly_file_path = data_root_dir + "/" + remote_assembly_file_path;
    QFile target_assembly_file(target_assembly_file_path);

    QString target_assembly_parameters_file_path = data_root_dir + "/" + remote_assembly_parameters_file_path;
    QFile target_assembly_parameters_file(target_assembly_parameters_file_path);

    QString source_assembly_file_path = temp_import_dir_path + "/" + remote_assembly_file_path;
    QFile source_assembly_file(source_assembly_file_path);

    QString source_assembly_parameters_file_path = temp_import_dir_path + "/" + remote_assembly_parameters_file_path;
    QFile source_assembly_parameters_file(source_assembly_parameters_file_path);

    bool remove_target_job_file = false;
    bool remove_target_job_parameters_file = false;
    bool remove_target_assembly_file = false;
    bool remove_target_assembly_parameters_file = false;
    bool import_assembly_files = false;

    QFile target_job_file;
    QFile target_job_parameters_file;

    qDebug() << "Checking if job/assembly are already present on local platform...";

    if (_is_job_import_action) {
        target_job_file_path = data_root_dir + "/" + remote_job_file;
        target_job_file.setFileName(target_job_file_path);
        target_job_parameters_file_path = data_root_dir + "/" + remote_job_parameters_file;
        target_job_parameters_file.setFileName(target_job_parameters_file_path);

        if (target_job_file.exists()) {
            qDebug() << "Job already present on local platform";
            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("Task %1 already exists.\n")
                        .arg(job_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }

            remove_target_job_file = true;

            if (target_job_parameters_file.exists()) {
                qDebug() << "Job parameters file already present on local platform";
                remove_target_job_parameters_file = true;
            } else {
                /* Technically unconsistent case : the job exists but no job parameters file was found */
                qWarning() << QString("The job '%1' exists on local platform but the job parameters file was not found").arg(job_name);
            }

        } else { // Job file does not exist on local platform

            if (target_job_parameters_file.exists()) {
                /* Technically unconsistent case : the job does not exist on local platform but a job parameters file was found with a corresponding name */
                qWarning() << QString("The job parameters file for job '%1' was found on local platform but job definition file does not exist").arg(job_name);
                remove_target_job_parameters_file = true;
            }
        }

        if (!target_assembly_file.exists()) {
            /* Technically unconsistent case : the assembly was selected but its file does not exist */
            qCritical() << QString("The assembly definition file for assembly '%1' was not found on local platform, job import failed").arg(_current_assembly_name);
            return false;
        }

        // check that assembly and assembly parameters file are identical
        if (FileUtils::areFilesIdentical(source_assembly_file_path, target_assembly_file_path)) {
            qDebug() << "Remote and local assembly definition files are identical";
        } else {
            qDebug() << "Remote and local assembly definition files are different";
            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("Processing chain %1 in archive is different from the one on your computer.\n")
                        .arg(_current_assembly_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }
        }

        remove_target_assembly_file = true;
        import_assembly_files = true;

        if (target_assembly_parameters_file.exists()) {
            if (FileUtils::areFilesIdentical(source_assembly_parameters_file_path, target_assembly_parameters_file_path)) {
                qDebug() << "Remote and local assembly parameters files are identical";
            } else {
                qDebug() << "Remote and local assembly parameters files are different";
                if (QMessageBox::No == QMessageBox::question(
                            m_parent_widget,
                            import_title,
                            tr("Parameters from processing chain %1 in archive is different from the one on your computer.\n")
                            .arg(_current_assembly_name).append(are_you_sure_replace_message),
                            QMessageBox::Yes,
                            QMessageBox::No)) {
                    return false;
                }
            }

            remove_target_assembly_parameters_file = true;

        } else {
            /* Technically unconsistent case : the assembly exists but no assembly parameters file was found */
            qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(_current_assembly_name);
            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("Processing chain %1 exists on this computer but has no parameters file.\n")
                        .arg(_current_assembly_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }
        }

    } else { // Assembly import action

        import_assembly_files = true;

        if (target_assembly_file.exists()) {
            qDebug() << "Assembly already present on local platform";
            assembly_name = process_data_manager->getAssemblyNameByFilePath(target_assembly_file_path);

            if (QMessageBox::No == QMessageBox::question(
                        m_parent_widget,
                        import_title,
                        tr("Processing chain %1 already exists on this computer.\n")
                        .arg(assembly_name).append(are_you_sure_replace_message),
                        QMessageBox::Yes,
                        QMessageBox::No)) {
                return false;
            }

            remove_target_assembly_file = true;
        }

        if (target_assembly_parameters_file.exists()) {
            if (target_assembly_file.exists()) {
                qDebug() << "Assembly parameters file already present on local platform";
            } else {
                /* Technically unconsistent case */
                qWarning() << QString("The assembly parameters file for assembly '%1' was found on local platform but assembly definition file does not exist").arg(assembly_name);
            }

            remove_target_assembly_parameters_file = true;
        }

    } // end if (isJobImportAction)


    /* REMOVING FILES TO BE REPLACED */
    qDebug() << "Removing existing assembly/job files...";

    if (remove_target_job_file) {
        if (!target_job_file.remove()) {
            qCritical() << QString("Could not remove existing job file '%1'.").arg(target_job_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not remove existing job file '%1'.\n")
                                  .arg(target_job_file_path).append(remove_file_manually));
            return false;
        }
    }

    if (remove_target_job_parameters_file) {
        if (!target_job_parameters_file.remove()) {
            qCritical() << QString("Could not remove existing job parameters file '%1'.").arg(target_job_parameters_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not remove existing job parameters file '%1'.\n")
                                  .arg(target_job_parameters_file_path).append(remove_file_manually));
            return false;
        }
    }

    if (remove_target_assembly_file) {
        if (!target_assembly_file.remove()) {
            qCritical() << QString("Could not remove existing assembly file '%1'.").arg(target_assembly_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not remove existing assembly file '%1'.\n")
                                  .arg(target_assembly_file_path).append(remove_file_manually));
            return false;
        }
    }

    if (remove_target_assembly_parameters_file) {
        if (!target_assembly_parameters_file.remove()) {
            qCritical() << QString("Could not remove existing assembly parameters file '%1'.").arg(target_assembly_parameters_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not remove existing assembly parameters file '%1'.\n")
                                  .arg(target_assembly_parameters_file_path).append(remove_file_manually));
            return false;
        }
    }


    /* PROCEEDING WITH IMPORT */
    qDebug() << "Proceeding with assembly/job import...";
    if (_is_job_import_action) {
        QFile source_job_file(source_job_file_path);

        qDebug() << "Copying job file...";
        if (!source_job_file.copy(target_job_file_path)) {
            qCritical() << QString("Could not copy job file '%1' to '%2'.").arg(source_job_file_path).arg(target_job_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not copy job file"));
            return false;
        } else {
            qDebug() << "... done";
        }

        QString source_job_parameters_file_path = temp_import_dir_path + "/" + remote_job_parameters_file;
        QFile source_job_parameters_file(source_job_parameters_file_path);

        qDebug() << "Copying job parameters file...";
        if (!source_job_parameters_file.copy(target_job_parameters_file_path)) {
            qCritical() << QString("Could not copy job parameters file '%1' to '%2'.").arg(source_job_file_path).arg(target_job_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not copy job parameters file"));
            return false;
        } else {
            qDebug() << "... done";
        }
    }

    // Assembly files are copied in case of assembly import, or job import with user confirmation for replacement
    if (import_assembly_files) {
        qDebug() << "Copying assembly file...";
        if (!source_assembly_file.copy(target_assembly_file_path)) {
            qCritical() << QString("Could not copy assembly file '%1' to '%2'.").arg(source_assembly_file_path).arg(target_assembly_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not copy assembly file"));
            return false;
        } else {
            qDebug() << "... done";
        }

        qDebug() << "Copying assembly parameters file...";
        if (!source_assembly_parameters_file.copy(target_assembly_parameters_file_path)) {
            qCritical() << QString("Could not copy assembly parameters file '%1' to '%2'.").arg(source_assembly_file_path).arg(target_assembly_file_path);
            QMessageBox::critical(m_parent_widget, import_failed_title, tr("Could not copy assembly parameters file"));
            return false;
        } else {
            qDebug() << "... done";
        }
    }

    /* Displaying success message */
    QString success_message = "";

    if (_is_job_import_action) {
        if (job_name.isEmpty()) {
            job_name = process_data_manager->getJobNameByFilePath(source_job_file_path);
        }

        success_message = tr("Task '%1' successfully imported.").arg(job_name);
    } else {
        if (assembly_name.isEmpty()) {
            assembly_name = process_data_manager->getAssemblyNameByFilePath(source_assembly_file_path);
        }

        success_message = tr("Processing chain '%1' successfully imported.").arg(assembly_name);
    }

    QMessageBox::information(
                m_parent_widget,
                import_title,
                success_message);

    return true;
}

void ImportExportHelper::checkImportExportDirs()
{
    bool already_checked = !m_export_path.isEmpty();

    if (already_checked) {
      return;
    }

    QString import_export_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
            QDir::separator() + "exchange";

    m_export_path = import_export_dir + QDir::separator() + "export";

    QDir export_dir(m_export_path);
    if (!export_dir.exists()) {
        qDebug() << "Creating export directory " << m_export_path;
        export_dir.mkpath(".");
    }

    m_import_path = import_export_dir + QDir::separator() + "import";

    QDir importDir(m_import_path);
    if (!importDir.exists()) {
        qDebug() << "Creating import directory " << m_import_path;
        importDir.mkpath(".");
    }
}

} // namespace matisse
