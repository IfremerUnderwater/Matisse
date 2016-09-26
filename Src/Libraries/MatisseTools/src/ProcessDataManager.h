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

#include "KeyValueList.h"
#include "AssemblyDefinition.h"
#include "JobDefinition.h"

namespace MatisseTools {

///
/// \brief The ProcessDataManager class is used for reading writing business data for assemblies, jobs and parameters
///


class ProcessDataManager
{
public:
    ProcessDataManager(QString dataRootDir, QString userDataPath);

    void loadAssembliesAndJobs();
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
    void clearAssemblies();
    void clearJobs();
    void clearArchivedJobs();
    bool loadModels();
    bool xmlIsValid(QXmlSchema & schema, QFileInfo fileInfo);
    bool duplicateElementParameters(QString fileName, QString newFileName, bool isAssembly);
    bool restoreArchivedJob(QString archivePath, QString jobToRestore);

    QString _basePath;

    QXmlSchema _assembliesSchema;

    QString _jobsParametersPath;
    QString _jobsPath;
    QString _assembliesPath;
    QString _assembliesParametersPath;
    QString _schemaPath; // TODO provisoire

    QString _assemblyFilePattern;
    QString _assemblyParametersFilePattern;
    QString _jobFilePattern;
    QString _jobParametersFilePattern;

    QHash<QString, AssemblyDefinition*> _assemblies;
    QHash<QString, JobDefinition*> _jobs;
    QMap<QString, JobDefinition*> _archivedJobs;

    QMap<QString, QString> _assemblyNameByFileName;
    QMap<QString, QString> _jobNameByFileName;
    QMap<QString, QString> _archivedJobNameByFileName;
    QMap<QString, QStringList*> _archivedJobsByAssembly;

    static const QString RELATIVE_EXECUTION_LOG_PATH;
};
}

#endif // PROCESS_DATA_MANAGER_H
