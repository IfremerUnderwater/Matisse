#include "AssemblyDefinition.h"
#include "JobDefinition.h"

using namespace MatisseTools;

JobDefinition::JobDefinition(QString name, QString assemblyName, QString assemblyVersion)
    : _name(name),
      _comment(""),
      _assemblyName(assemblyName),
      _assemblyVersion(assemblyVersion),
      _executionDefinition(NULL),
      _parametersDefinition(NULL)
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

ParameterDefinition *JobDefinition::parametersDefinition() const
{
    return _parametersDefinition;
}

void JobDefinition::setParametersDefinition(ParameterDefinition *parametersDefinition)
{
    _parametersDefinition = parametersDefinition;
}
ExecutionDefinition *JobDefinition::executionDefinition() const
{
    return _executionDefinition;
}

void JobDefinition::setExecutionDefinition(ExecutionDefinition *executionDefinition)
{
    _executionDefinition = executionDefinition;
}

QString JobDefinition::serialized()
{
    // NB: on ne teste pas parametersDefinition() car on en a forcément un à l'écriture...
    QString text;
    QString dateStr;
    QString executedStr = "false";
    QString resultFile = "";
    if (executionDefinition()) {
        QDateTime date(executionDefinition()->executionDate());
        if (date.isValid()) {
            dateStr = date.toString("dd/MM/yyyy hh:mm");
        }
        if (executionDefinition()->executed()) {
            executedStr = "true";
        }
        resultFile = executionDefinition()->resultFileName();
    }

    text.append(QString("<MatisseJob name=\"%1\" assembly=\"%2\" version=\"%3\">\n").arg(name()).arg(assemblyName()).arg(assemblyVersion()));
    text.append(QString("\t<Comments>%1</Comments>\n").arg(comment()));
    text.append(QString("\t<Parameters model=\"%1\" name=\"%2\"/>\n").arg(parametersDefinition()->model()).arg(parametersDefinition()->name()));
    text.append(QString("\t<Execution executed=\"%1\" executionDate=\"%2\" result=\"%3\"/>\n").arg(executedStr).arg(dateStr).arg(resultFile));
    text.append(QString("</MatisseJob>\n"));

    return text;
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



QString ExecutionDefinition::resultFileName() const
{
    return _resultFileName;
}

void ExecutionDefinition::setResultFileName(const QString &resultFileName)
{
    _resultFileName = resultFileName;
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

void ExecutionDefinition::setExecutionDate(const QDateTime &executionDate)
{
    _executionDate = executionDate;
}


