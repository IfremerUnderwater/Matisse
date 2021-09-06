#ifndef MATISSE_JOB_DEFINITION_H_
#define MATISSE_JOB_DEFINITION_H_

#include "assembly_definition.h"

///
/// Business object for jobs
///


namespace matisse {

class ExecutionDefinition {
    friend class JobDefinition;
public:
    ExecutionDefinition() {}
    virtual ~ExecutionDefinition() {}

    QStringList resultFileNames() const;
    void setResultFileNames(QStringList _result_file_names);

    bool executed() const;
    void setExecuted(bool _executed);

    QDateTime executionDate() const;
    void setExecutionDate(QDateTime _execution_date);

private:
    QDateTime  m_execution_date;
    bool m_executed;
    QStringList m_result_file_names;
};

class RemoteJobDefinition {

public:
  RemoteJobDefinition() {}

  bool isScheduled() const;
  void setScheduled(bool _scheduled);

  QString node() const;
  void setNode(QString _node);

  int jobId() const;
  void setJobId(int _job_id);

  QDateTime timestamp() const;
  void setTimestamp(QDateTime _timestamp);

private:
  bool m_is_scheduled = false;
  QString m_node;
  int m_job_id = -1;
  QDateTime m_timestamp;
};

class JobDefinition
{
public:
    JobDefinition(QString _name = "", QString _assembly_name = "", QString _assembly_version = "");

    virtual ~JobDefinition() {}

    QString filename() const;
    void setFilename(const QString &_filename);

    QString name() const;
    void setName(const QString &_name);

    QString assemblyName() const;
    QString assemblyVersion() const;

    QString comment() const;
    void setComment(const QString &_comment);

    ExecutionDefinition *executionDefinition() const;
    void setExecutionDefinition(ExecutionDefinition *_execution_definition);

    RemoteJobDefinition* remoteJobDefinition() const;
    void setRemoteJobDefinition(RemoteJobDefinition* _remote_job_definition);

    JobDefinition *duplicate(QString _new_name, QString _new_file_name);

private:
    QString m_filename;
    QString m_name;
    QString m_comment;
    QString m_assembly_name;
    QString m_assembly_version;
    ExecutionDefinition *m_execution_definition;
    RemoteJobDefinition* m_remote_job_definition;
};

} // namespace matisse

#endif // MATISSE_JOB_DEFINITION_H_
