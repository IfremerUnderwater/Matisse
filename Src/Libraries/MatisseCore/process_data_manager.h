#ifndef MATISSE_PROCESS_DATA_MANAGER_H_
#define MATISSE_PROCESS_DATA_MANAGER_H_

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

namespace matisse {

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
    void loadArchivedJobs(QString _archive_path);

    bool readAssemblyFile(QString _filename);
    AssemblyDefinition *getAssembly(QString _assembly_name);

    bool readJobFile(QString _filename, bool _is_archive_file = false, QString _archive_path = "");
    bool writeJobFile(JobDefinition *_job, bool _overwrite = false);
    bool writeAssemblyFile(AssemblyDefinition *_assembly, bool _overwrite = false);


    JobDefinition *getJob(QString _job_name);
    JobDefinition *getArchivedJob(QString _archived_job_name);
    QStringList getAssemblyArchivedJobs(QString _assembly_name) const;
    QStringList getArchivedJobNames() const;
    bool restoreArchivedJobs(QString _archive_path, QString _assembly_name, QStringList _jobs_to_restore);
    QStringList getJobsNames();
    QString getJobsBasePath();
    QString getJobParametersFilePath(QString _job_name);

    QStringList getAssembliesList();
    QString getAssembliesPath();
    QString getAssembliesParametersPath() const;


    QString getAssemblyFilePattern() const;
    QString getAssemblyParametersFilePattern() const;
    QString getJobFilePattern() const;
    QString getJobParametersFilePattern() const;

    QString getAssemblyNameByFilePath(QString _filename) const;
    QString getJobNameByFilePath(QString _filename) const;

    QString getJobsParametersBasePath() const;

    bool copyJobFilesToResult(QString _job_name, QString _result_path);
    bool archiveJobFiles(QString _job_name, QString _archive_path);
    bool duplicateJob(QString _job_name, QString _new_name);
    bool duplicateAssembly(QString _assembly_name, QString _new_name);

    QString fromNameToFileName(QString _name);
    bool assemblyHasArchivedJob(QString _assembly_name);

private:
    ProcessDataManager();
    ~ProcessDataManager();
    void clearAssemblies();
    void clearJobs();
    void clearArchivedJobs();
    bool loadModels();
    bool xmlIsValid(QXmlSchema & _schema, QFileInfo _file_info);
    bool duplicateElementParameters(QString _filename, QString _new_filename, bool _is_assembly);
    bool restoreArchivedJob(QString _archive_path, QString _job_to_restore);

    static ProcessDataManager* m_temp_instance;

    QString m_base_path;

    QXmlSchema m_assemblies_schema;

    QString m_jobs_parameters_path;
    QString m_jobs_path;
    QString m_assemblies_path;
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

} // namespace matisse

#endif // MATISSE_PROCESS_DATA_MANAGER_H_
