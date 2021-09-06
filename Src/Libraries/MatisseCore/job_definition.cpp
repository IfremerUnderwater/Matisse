#include "assembly_definition.h"
#include "job_definition.h"

namespace matisse {

JobDefinition::JobDefinition(QString _name, QString _assembly_name, QString _assembly_version)
    : m_name(_name),
      m_comment(""),
      m_assembly_name(_assembly_name),
      m_assembly_version(_assembly_version),
      m_execution_definition(NULL),
      m_remote_job_definition(NULL)
{
}

QString JobDefinition::filename() const
{
    return m_filename;

}

void JobDefinition::setFilename(const QString &_filename)
{
    m_filename = _filename;
}

ExecutionDefinition *JobDefinition::executionDefinition() const
{
    return m_execution_definition;
}

void JobDefinition::setExecutionDefinition(ExecutionDefinition* _execution_definition)
{
  m_execution_definition = _execution_definition;
}

void JobDefinition::setRemoteJobDefinition(RemoteJobDefinition *_remote_job_definition)
{
    m_remote_job_definition = _remote_job_definition;
}

RemoteJobDefinition* JobDefinition::remoteJobDefinition() const
{
  return m_remote_job_definition;
}


JobDefinition *JobDefinition::duplicate(QString _new_name, QString _new_file_name)
{
    JobDefinition *new_job = new JobDefinition(_new_name, m_assembly_name, m_assembly_version);
    new_job->setFilename(_new_file_name);
    new_job->setComment(m_comment);
    ExecutionDefinition *execution_def = new ExecutionDefinition();
    execution_def->setExecuted(false);
    new_job->setExecutionDefinition(execution_def);
    RemoteJobDefinition* remote_def = new RemoteJobDefinition();
    remote_def->setScheduled(false);
    new_job->setRemoteJobDefinition(remote_def);

    return new_job;
}


QString JobDefinition::name() const
{
    return m_name;
}

void JobDefinition::setName(const QString &_name)
{
    m_name = _name;
}

QString JobDefinition::assemblyVersion() const
{
    return m_assembly_version;
}

QString JobDefinition::assemblyName() const
{
    return m_assembly_name;
}

void JobDefinition::setComment(const QString &_comment)
{
    m_comment = _comment;
}

QString JobDefinition::comment() const
{
    return m_comment;
}



QStringList ExecutionDefinition::resultFileNames() const
{
    return m_result_file_names;
}

void ExecutionDefinition::setResultFileNames(QStringList _result_file_names)
{
    m_result_file_names = _result_file_names;
}
bool ExecutionDefinition::executed() const
{
    return m_executed;
}

void ExecutionDefinition::setExecuted(bool _executed)
{
    m_executed = _executed;
}
QDateTime ExecutionDefinition::executionDate() const
{
    return m_execution_date;
}

void ExecutionDefinition::setExecutionDate(QDateTime _execution_date)
{
    m_execution_date = _execution_date;
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

} // namespace matisse
