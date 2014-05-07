﻿#ifndef JOBDEFINITION_H
#define JOBDEFINITION_H

#include "AssemblyDefinition.h"

namespace MatisseTools {
class ParameterDefinition;

class ExecutionDefinition {
    friend class JobDefinition;
public:
    ExecutionDefinition() {}
    virtual ~ExecutionDefinition() {}

    QString resultFileName() const;
    void setResultFileName(const QString &resultFileName);

    bool executed() const;
    void setExecuted(bool executed);

    QDateTime executionDate() const;
    void setExecutionDate(const QDateTime &executionDate);

private:
    QDateTime  _executionDate;
    bool _executed;
    QString _resultFileName;
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

    ParameterDefinition *parametersDefinition() const;
    void setParametersDefinition(ParameterDefinition *parametersDefinition);

    ExecutionDefinition *executionDefinition() const;
    void setExecutionDefinition(ExecutionDefinition *executionDefinition);

    QString serialized();

private:
    QString _filename;
    QString _name;
    QString _comment;
    QString _assemblyName;
    QString _assemblyVersion;
    ExecutionDefinition *_executionDefinition;
    ParameterDefinition *_parametersDefinition;
};
}

#endif // JOBDEFINITION_H
