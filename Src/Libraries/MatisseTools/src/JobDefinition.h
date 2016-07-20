#ifndef JOBDEFINITION_H
#define JOBDEFINITION_H

#include "AssemblyDefinition.h"

///
/// Business object for jobs
///


namespace MatisseTools {

class ExecutionDefinition {
    friend class JobDefinition;
public:
    ExecutionDefinition() {}
    virtual ~ExecutionDefinition() {}

    QStringList resultFileNames() const;
    void setResultFileNames(QStringList resultFileNames);

    bool executed() const;
    void setExecuted(bool executed);

    QDateTime executionDate() const;
    void setExecutionDate(QDateTime executionDate);

private:
    QDateTime  _executionDate;
    bool _executed;
    QStringList _resultFileNames;
};

class JobDefinition
{
public:
    JobDefinition(QString name = "", QString assemblyName = "", QString assemblyVersion = "");

    virtual ~JobDefinition() {}

    QString filename() const;
    void setFilename(const QString &filename);

    QString name() const;
    void setName(const QString &name);

    QString assemblyName() const;
    QString assemblyVersion() const;

    QString comment() const;
    void setComment(const QString &comment);

    ExecutionDefinition *executionDefinition() const;
    void setExecutionDefinition(ExecutionDefinition *executionDefinition);

    JobDefinition *duplicate(QString newName, QString newFileName);

private:
    QString _filename;
    QString _name;
    QString _comment;
    QString _assemblyName;
    QString _assemblyVersion;
    ExecutionDefinition *_executionDefinition;
};
}

#endif // JOBDEFINITION_H
