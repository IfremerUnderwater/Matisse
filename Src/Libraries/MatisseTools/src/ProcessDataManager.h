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

class ProcessDataManager
{
public:
    ProcessDataManager(QString dataRootDir, QString userDataPath);

    bool readAssemblyFile(QString filename);
    AssemblyDefinition *getAssembly(QString assemblyName);

    bool readJobFile(QString filename);
    bool writeJobFile(JobDefinition *job, bool overWrite = false);
    bool saveAssembly(QString filename, AssemblyDefinition *assembly);

    JobDefinition *getJob(QString jobName);
    QStringList getJobsNames();
    QString getJobsPath();
    QString getJobParametersFilePath(QString jobName);

    QStringList getAssembliesList();
    QString getAssembliesPath();
    QString getAssembliesParametersPath() const;

    void clearAssemblies();
    void clearJobs();

    QString getAssemblyFilePattern() const;
    QString getAssemblyParametersFilePattern() const;
    QString getJobFilePattern() const;
    QString getJobParametersFilePattern() const;

    QString getAssemblyNameByFilePath(QString fileName) const;
    QString getJobNameByFilePath(QString fileName) const;

private:
    bool loadModels();
    bool xmlIsValid(QXmlSchema & schema, QFileInfo fileInfo);

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

    QMap<QString, QString> _assemblyNameByFileName;
    QMap<QString, QString> _jobNameByFileName;
};
}

#endif // PROCESS_DATA_MANAGER_H
