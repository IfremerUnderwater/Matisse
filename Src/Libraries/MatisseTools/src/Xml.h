﻿#ifndef XML_H
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
#include "MatissePreferences.h"

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
    bool writeJobFile(JobDefinition *job, bool overWrite = false);
    bool readMatisseGuiSettings(QString filename);
    bool readMatissePreferences(QString filename, MatissePreferences &prefs);
    bool writeMatissePreferences(QString filename, MatissePreferences &prefs);
    bool saveAssembly(QString filename, AssemblyDefinition *assembly);

    JobDefinition *getJob(QString jobName);
    QStringList getJobsNames();
    QString getJobsPath();
    QString getJobsParametersPath(QString jobName);

    QStringList getAssembliesList();
    QString getAssembliesPath();
    QString getAssembliesParametersPath() { return _assembliesParametersPath; }

    void clearAssemblies();
    void clearJobs();

    QString getVersion() const;

private:
    bool loadModels();
    bool xmlIsValid(QXmlSchema & schema, QFileInfo fileInfo);

    QString _basePath;
    QString _dllPath;
    int _port;
    QString _version;

    QXmlSchema _assembliesSchema;

    QString _jobsParametersPath;
    QString _jobsPath;
    QString _assembliesPath;
    QString _assembliesParametersPath;

    QHash<QString, AssemblyDefinition*> _assemblies;
    QHash<QString, JobDefinition*> _jobs;

};
}

#endif // XML_H
