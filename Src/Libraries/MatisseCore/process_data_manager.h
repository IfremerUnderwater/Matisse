#ifndef PROCESS_DATA_MANAGER_H
#define PROCESS_DATA_MANAGER_H

#include <QString>
#include <QStringList>
#include <QDate>
#include <QHash>
#include <QFile>
#include <QFileInfo>
#include <QByteArray>
#include <QBuffer>
#include <QtXml>
#include <QtXmlPatterns>
#include <QXmlAttributes>
#include <QtDebug>

#include "key_value_list.h"
#include "assembly_definition.h"
#include "job_definition.h"

namespace MatisseTools {

///
/// \brief The ProcessDataManager class is used for reading writing business data for assemblies, jobs and parameters
///


class ProcessDataManager
{
public:
    static ProcessDataManager* instance();
    static ProcessDataManager* newTempInstance();
    static void deleteTempInstance();

    ProcessDataManager(ProcessDataManager const&) = delete;        // Disable copy
    void operator=(ProcessDataManager const&) = delete;   // Disable copy

    void init(QString _data_root_dir, QString _user_data_path);


    bool loadAssembliesAndJobs();
    void loadArchivedJobs(QString archivePath);

    bool readAssemblyFile(QString filename);
    AssemblyDefinition *getAssembly(QString assemblyName);

    bool readJobFile(QString filename, bool isArchiveFile = false, QString archivePath = "");
    bool writeJobFile(JobDefinition *job, bool overWrite = false);
    bool writeAssemblyFile(AssemblyDefinition *assembly, bool overWrite = false);


    JobDefinition *getJob(QString jobName);
    JobDefinition *getArchivedJob(QString archivedJobName);
    QStringList getAssemblyArchivedJobs(QString assemblyName) const;
    QStringList getArchivedJobNames() const;
    bool restoreArchivedJobs(QString archivePath, QString assemblyName, QStringList jobsToRestore);
    QStringList getJobsNames();
    QString getJobsBasePath();
    QString getJobParametersFilePath(QString jobName);

    QStringList getAssembliesList();
    QString getAssembliesPath();
    QString getAssembliesParametersPath() const;


    QString getAssemblyFilePattern() const;
    QString getAssemblyParametersFilePattern() const;
    QString getJobFilePattern() const;
    QString getJobParametersFilePattern() const;

    QString getAssemblyNameByFilePath(QString fileName) const;
    QString getJobNameByFilePath(QString fileName) const;

    QString getJobsParametersBasePath() const;

    bool copyJobFilesToResult(QString jobName, QString resultPath);
    bool archiveJobFiles(QString jobName, QString archivePath);
    bool duplicateJob(QString jobName, QString newName);
    bool duplicateAssembly(QString assemblyName, QString newName);

    QString fromNameToFileName(QString name);
    bool assemblyHasArchivedJob(QString assemblyName);

private:
    ProcessDataManager();
    ~ProcessDataManager();
    void clearAssemblies();
    void clearJobs();
    void clearArchivedJobs();
    bool loadModels();
    bool xmlIsValid(QXmlSchema & schema, QFileInfo fileInfo);
    bool duplicateElementParameters(QString fileName, QString newFileName, bool isAssembly);
    bool restoreArchivedJob(QString archivePath, QString jobToRestore);

    static ProcessDataManager* m_temp_instance;

    QString m_base_path;

    QXmlSchema m_assemblies_schema;

    QString m_jobs_parameters_path;
    QString m_jobs_path;
    QString m__assemblies_path;
    QString m_assemblies_parameters_path;
    QString m_schema_path; // TODO provisoire

    QString m_assembly_file_pattern;
    QString m_assembly_parameters_file_pattern;
    QString m_job_file_pattern;
    QString m_job_parameters_file_pattern;

    QHash<QString, AssemblyDefinition*> m_assemblies;
    QHash<QString, JobDefinition*> m_jobs;
    QMap<QString, JobDefinition*> m_archived_jobs;

    QMap<QString, QString> m_assembly_name_by_file_name;
    QMap<QString, QString> m_job_name_by_file_name;
    QMap<QString, QString> m_archived_job_name_by_file_name;
    QMap<QString, QStringList*> m_archived_jobs_by_assembly;

    static const QString RELATIVE_EXECUTION_LOG_PATH;
};
}

#endif // PROCESS_DATA_MANAGER_H
