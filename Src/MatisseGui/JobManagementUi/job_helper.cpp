#include <QtDebug>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>

#include "duplicate_dialog.h"
#include "iconized_tree_item_wrapper.h"
#include "job_dialog.h"
#include "job_helper.h"
#include "matisse_tree_item.h"
#include "parameters_common.h"
#include "process_data_manager.h"
#include "restore_jobs_dialog.h"

namespace matisse {

JobHelper::JobHelper(QObject *_parent) : QObject(_parent)
{
    m_parent_widget = static_cast<QWidget*>(_parent);
}

void JobHelper::checkArchiveDirCreated()
{
    bool already_checked = !m_archive_path.isEmpty();

    if (already_checked) {
      return;
    }

    m_archive_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
      QDir::separator() + "archive";

    QDir archive_dir(m_archive_path);
    if (!archive_dir.exists()) {
        qDebug() << "Creating archive directory " << m_archive_path;
        archive_dir.mkpath(".");
    }
}

QString JobHelper::archivePath()
{
    checkArchiveDirCreated();
    return m_archive_path;
}

bool JobHelper::archiveJob(JobDefinition *_job)
{
    checkArchiveDirCreated();

    bool archived = ProcessDataManager::instance()->archiveJobFiles(_job->name(), m_archive_path);

    if (archived) {
        QMessageBox::information(m_parent_widget, tr("Backup"), tr("Task '%1' has been backed up")
                                 .arg(_job->name()));

    } else {
        QMessageBox::critical(m_parent_widget, tr("Backup failure"), tr("Task '%1' couldn't be backed up.")
                              .arg(_job->name()));
    }

    return archived;
}

bool JobHelper::restoreJobs(AssemblyDefinition *_assembly)
{
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString assembly_name = _assembly->name();
    QStringList archived_jobs = process_data_manager->getAssemblyArchivedJobs(assembly_name);
    QStringList jobs_to_restore;

    RestoreJobsDialog dialog(m_parent_widget, assembly_name, archived_jobs, jobs_to_restore);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    if (jobs_to_restore.isEmpty()) {
        return false;
    }

    bool restored = process_data_manager->restoreArchivedJobs(m_archive_path, assembly_name, jobs_to_restore);
    return restored;
}

bool JobHelper::duplicateJob(JobDefinition *_job)
{
    QString job_name = _job->name();
    QString new_job_name;

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    QStringList existing_job_names = process_data_manager->getJobsNames();
    QStringList archived_job_names = process_data_manager->getArchivedJobNames();

    DuplicateDialog dialog(m_parent_widget, job_name, new_job_name, false, existing_job_names, archived_job_names);
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return false;
    }

    qDebug() << QString("Duplicating job %1 to %2").arg(job_name, new_job_name);
    bool duplicated = process_data_manager->duplicateJob(job_name, new_job_name);

    if (duplicated) {
        QMessageBox::information(m_parent_widget, tr("Task copy"), tr("Task '%1' copied")
                                 .arg(job_name));
    } else {
        QMessageBox::critical(m_parent_widget, tr("Copy failure"), tr("Task '%1' was not copied.")
                              .arg(job_name));
    }

    return duplicated;
}

JobDefinition * JobHelper::createNewJob(QString _assembly_name)
{
    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    AssemblyDefinition * assembly = process_data_manager->getAssembly(_assembly_name);

    if (!assembly) {
        qCritical() << QString("Assembly '%1' not found in repository").arg(_assembly_name);
        return nullptr;
    }

    qDebug() << "Creating new job";
    KeyValueList kvl;
    kvl.append("name");
    kvl.append("comment");

    kvl.append(DATASET_PARAM_DATASET_DIR);
    kvl.append(DATASET_PARAM_NAVIGATION_FILE);

    /* Key value list is initialized with default dataset preferences */
    kvl.set(DATASET_PARAM_OUTPUT_DIR, m_preferences->defaultResultPath());
    kvl.set(DATASET_PARAM_OUTPUT_FILENAME, m_preferences->defaultMosaicFilenamePrefix());

    /* Dataset parameters are loaded / overriden from template assembly parameters */
    m_engine->parametersManager()->pullDatasetParameters(kvl);

    QString jobs_base_path = process_data_manager->getJobsBasePath();
    QStringList job_names = process_data_manager->getJobsNames();
    QStringList archived_job_names = process_data_manager->getArchivedJobNames();

    JobDialog dialog(m_parent_widget, m_icon_factory, &kvl, jobs_base_path, job_names, archived_job_names);
    dialog.setFixedHeight(GraphicalCharter::instance().dpiScaled(JD_HEIGHT));
    dialog.setFixedWidth(GraphicalCharter::instance().dpiScaled(JD_WIDTH));
    dialog.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    if (dialog.exec() != QDialog::Accepted) {
        return nullptr;
    }

    QString assembly_version = assembly->version().simplified();

    QString job_name = kvl.getValue("name");

    JobDefinition new_job(job_name, _assembly_name, assembly_version);
    new_job.setComment(kvl.getValue("comment"));

    ExecutionDefinition execution_definition;
    execution_definition.setExecuted(false);
    new_job.setExecutionDefinition(&execution_definition);

    RemoteJobDefinition remote_job_definition;
    remote_job_definition.setScheduled(false);
    new_job.setRemoteJobDefinition(&remote_job_definition);

    if (!process_data_manager->writeJobFile(&new_job)) {
        qCritical() << QString("Job definition file could not be created for new job '%1'").arg(job_name);
        return nullptr;
    }

    m_engine->parametersManager()->createJobParametersFile(_assembly_name, job_name, kvl);

    // load job definition to local repository
    QString job_filename = job_name;
    job_filename.append(".xml");
    process_data_manager->readJobFile(job_filename);
    JobDefinition* new_job_def = process_data_manager->getJob(job_name);

    return new_job_def;
}

bool JobHelper::saveJob(JobDefinition *_job)
{
    // one can save an already executed job : executed status is removed...
    if (_job->executionDefinition()->executed()) {
        if (QMessageBox::No == QMessageBox::question(m_parent_widget, tr("Processed job..."),
                                                     tr("Job already processed.\n Do you still want to reprocess ?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No)) {
            return false;
        }

        _job->executionDefinition()->setExecuted(false);
        _job->executionDefinition()->setExecutionDate(QDateTime());
        _job->executionDefinition()->setResultFileNames(QStringList() << "");
    }

    /* save job and parameters */
    if (!ProcessDataManager::instance()->writeJobFile(_job, true)) {
        QMessageBox::warning(m_parent_widget, tr("Job file..."), tr("File %1 could not be write...").arg(_job->name() + ".xml"));
        return false;
    }

    m_engine->parametersManager()->saveParametersValues(_job->name(), false);

    return true;
}

bool JobHelper::deleteJob(QString job_name)
{
    int ret = QMessageBox::question(m_parent_widget, tr("Delete Job"), tr("Do you want to delete Job %1?").arg(job_name), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (ret==QMessageBox::Cancel) {
        return false;
    }

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();

    QString filename = process_data_manager->getJob(job_name)->filename();
    // suppression fichier job

    filename = process_data_manager->getJobsBasePath() + QDir::separator() + filename;
    QFile file(filename);
    if (!file.exists()) {
        qCritical() << QString("Job file '%1' not found, impossible to remove").arg(filename);
        return false;
    }

    if (!file.remove()) {
        qCritical() << QString("Job file '%1' could not be removed").arg(filename);
        return false;
    }

    filename = process_data_manager->getJobParametersFilePath(job_name);

    QFile parameter_file(filename);

    if (!parameter_file.exists()) {
        qCritical() << QString("Job parameters file '%1' not found, impossible to remove").arg(filename);
    } else {
        if (!parameter_file.remove()) {
            qCritical() << QString("Job parameters file '%1' could not be removed").arg(filename);
        }
    }

    return true;
}

bool JobHelper::prepareJobForExecution(JobDefinition *_job, QString &_status_msg)
{
    ProcessDataManager * process_data_manager = ProcessDataManager::instance();

    QString job_name = _job->name();

    // Parameter saving is handled by parent UI in case of user modification, is it necessary to maintain this line ?
    m_engine->parametersManager()->saveParametersValues(job_name, false);

    QString assembly_name =_job->assemblyName();
    AssemblyDefinition * assembly_def = process_data_manager->getAssembly(assembly_name);
    if (!assembly_def) {
        // inconsistent case : aborting
        qCritical() << "Assembly error" << assembly_name;
        _status_msg = tr("Assembly error");
        return false;
    }

    QString job_filename = _job->filename();

    qDebug() << "Running job " << job_name;

    QString msg1=tr("File not found.");
    QString msg2=tr("Job file %1 could not be launched").arg(job_name);

    // reloading job from job file... (beware : this creates a new instance of the JobDefinition object,
    // making other references obsolete)
    if (!process_data_manager->readJobFile(job_filename)) {
        qCritical() << QString("Error reading job file for job '%1' before launch").arg(job_name);
        QMessageBox::information(m_parent_widget, msg1, msg2);
        _status_msg = msg1 + " " + msg2;
        return false;
    }

    // reloading pointer to JobDefinition object from local cache
    _job = process_data_manager->getJob(job_name);

    if (!_job) {
        // inconsistent case
        qCritical() << QString("Job '%1' could not be loaded properly before launch").arg(job_name);
        QMessageBox::information(m_parent_widget, msg1, msg2);
        _status_msg = msg1 + " " + msg2;
        return false;
    }

    /* Copy XML files to result path */
    QString result_path = m_engine->parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
    QDir result_path_dir(result_path);
    if (result_path_dir.isRelative())
    {
        QString data_path = m_engine->parametersManager()->getParameterValue(DATASET_PARAM_DATASET_DIR);
        result_path = data_path + QDir::separator() + result_path_dir.path();
    }

    if (!process_data_manager->copyJobFilesToResult(job_name, result_path)) {
        qWarning() << "Job file and job parameter file could not be copied to result dir";
    }

    _status_msg = tr("Job %1 running...").arg(job_name);
    return true;
}

bool JobHelper::jobProcessed(QString _job_name)
{
    qDebug() << QString("Job %1 done, updating job file and displaying results...").arg(_job_name);

    if (m_engine->errorFlag()) {
        return false;
    }

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    JobDefinition *job_def = process_data_manager->getJob(_job_name);

    if (!job_def->executionDefinition()->executed()) {
        qWarning() << "Job definition is not marked as executed";
    } else {
        if (job_def->executionDefinition()->resultFileNames().isEmpty()) {
            qWarning() << "Job definition has no result files";
        }
    }

    // debug
    qDebug() << "Job comment is : " << job_def->comment();

    QDateTime now = QDateTime::currentDateTime();
    job_def->executionDefinition()->setExecutionDate(now);
    process_data_manager->writeJobFile(job_def, true);

    return true;
}

} // namespace matisse
