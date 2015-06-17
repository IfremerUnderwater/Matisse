#ifndef XML_H
#define XML_H

#include <QtCore>
#include <QObject>
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


class Xml
{
public:
    Xml();

    int port() { return _port; }
    QString getBasePath() { return _basePath; }
    QString getDllPath() { return _dllPath; }


    bool readAssemblyFile(QString filename);
    AssemblyDefinition *getAssembly(QString assemblyName);

    bool readJobFile(QString filename);

    bool updateJobFile(QString jobName);
    bool writeJobFile(QString jobName, bool overWrite = false);
    bool writeJobFile(JobDefinition *job, bool overWrite = false);

    JobDefinition *getJob(QString jobName);
    QStringList getJobsNames();
    QString getModelPath(QString parameterVersion);
    QString getJobsPath();
    QString getJobsParametersPath(QString parameterVersion, QString parameterName);

    QString getAssembliesParametersPath(QString parameterVersion, QString parameterName);
    QStringList getAssembliesList();
    QString getAssembliesPath();
    bool readMatisseGuiSettings(QString filename);
    static bool validateXmlFile(QString xmlSchema, QString xmlFile = "");

    KeyValueList readParametersFileDescriptor(QString filename);
    void clearAssembliesDatas();
    void clearJobsDatas();

private:
    bool loadModels();
    bool xmlIsValid(QXmlSchema & schema, QFileInfo fileInfo);

    QString _basePath;
    QString _dllPath;
    int _port;
    QXmlSchema _assembliesSchema;

    QString _jobsParametersPath;
    QString _jobsPath;
    QString _assembliesPath;

    QHash<QString, AssemblyDefinition*> _assemblies;
    QHash<QString, JobDefinition*> _jobs;

};
}

#endif // XML_H
