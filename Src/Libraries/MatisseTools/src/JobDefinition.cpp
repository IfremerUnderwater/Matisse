#include "AssemblyDefinition.h"
#include "JobDefinition.h"

using namespace MatisseTools;

JobDefinition::JobDefinition(QString name, QString assemblyName, QString assemblyVersion)
    : _name(name),
      _comment(""),
      _assemblyName(assemblyName),
      _assemblyVersion(assemblyVersion),
      _executionDefinition(NULL)
{
}

QString JobDefinition::filename() const
{
    return _filename;

}

void JobDefinition::setFilename(const QString &filename)
{
    _filename = filename;
}

ExecutionDefinition *JobDefinition::executionDefinition() const
{
    return _executionDefinition;
}

void JobDefinition::setExecutionDefinition(ExecutionDefinition *executionDefinition)
{
    _executionDefinition = executionDefinition;
}

JobDefinition *JobDefinition::duplicate(QString newName, QString newFileName)
{
    JobDefinition *newJob = new JobDefinition(newName, _assemblyName, _assemblyVersion);
    newJob->setFilename(newFileName);
    newJob->setComment(_comment);
    ExecutionDefinition *executionDef = new ExecutionDefinition();
    executionDef->setExecuted(false);
    newJob->setExecutionDefinition(executionDef);

    return newJob;
}


QString JobDefinition::name() const
{
    return _name;
}

void JobDefinition::setName(const QString &name)
{
    _name = name;
}

QString JobDefinition::assemblyVersion() const
{
    return _assemblyVersion;
}

QString JobDefinition::assemblyName() const
{
    return _assemblyName;
}

void JobDefinition::setComment(const QString &comment)
{
    _comment = comment;
}

QString JobDefinition::comment() const
{
    return _comment;
}



QStringList ExecutionDefinition::resultFileNames() const
{
    return _resultFileNames;
}

void ExecutionDefinition::setResultFileNames(QStringList resultFileNames)
{
    _resultFileNames = resultFileNames;
}
bool ExecutionDefinition::executed() const
{
    return _executed;
}

void ExecutionDefinition::setExecuted(bool executed)
{
    _executed = executed;
}
QDateTime ExecutionDefinition::executionDate() const
{
    return _executionDate;
}

void ExecutionDefinition::setExecutionDate(QDateTime executionDate)
{
    _executionDate = executionDate;
}


