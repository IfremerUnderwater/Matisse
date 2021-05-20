#include "assembly_definition.h"
#include "job_definition.h"

using namespace MatisseTools;

JobDefinition::JobDefinition(QString name, QString assemblyName, QString assemblyVersion)
    : _name(name),
      _comment(""),
      _assemblyName(assemblyName),
      _assemblyVersion(assemblyVersion),
      _executionDefinition(NULL),
      m_remote_job_definition(NULL)
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

void JobDefinition::setExecutionDefinition(ExecutionDefinition* executionDefinition)
{
  _executionDefinition = executionDefinition;
}

void JobDefinition::setRemoteJobDefinition(RemoteJobDefinition *_remote_job_definition)
{
    m_remote_job_definition = _remote_job_definition;
}

RemoteJobDefinition* JobDefinition::remoteJobDefinition() const
{
  return m_remote_job_definition;
}


JobDefinition *JobDefinition::duplicate(QString newName, QString newFileName)
{
    JobDefinition *newJob = new JobDefinition(newName, _assemblyName, _assemblyVersion);
    newJob->setFilename(newFileName);
    newJob->setComment(_comment);
    ExecutionDefinition *executionDef = new ExecutionDefinition();
    executionDef->setExecuted(false);
    newJob->setExecutionDefinition(executionDef);
    RemoteJobDefinition* remote_def = new RemoteJobDefinition();
    remote_def->setScheduled(false);
    newJob->setRemoteJobDefinition(remote_def);

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


bool RemoteJobDefinition::isScheduled() const
{
  return m_is_scheduled;
}

void RemoteJobDefinition::setScheduled(bool _is_scheduled)
{
  m_is_scheduled = _is_scheduled;
}

QString RemoteJobDefinition::node() const {
  return m_node;
}

void RemoteJobDefinition::setNode(QString _node) {
  m_node = _node;
}

int RemoteJobDefinition::jobId() const {
  return m_job_id;
}

void RemoteJobDefinition::setJobId(int _job_id) {
  m_job_id = _job_id;
}

QDateTime RemoteJobDefinition::timestamp() const {
  return m_timestamp;
}

void RemoteJobDefinition::setTimestamp(QDateTime _timestamp) {
  m_timestamp = _timestamp;
}

