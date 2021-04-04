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

class RemoteExecutionDefinition {
  friend class JobDefinition;

 public:
  RemoteExecutionDefinition() {}
  virtual ~RemoteExecutionDefinition() {}

  QStringList resultFileNames() const;
  void setResultFileNames(QStringList resultFileNames);

  bool executed() const;
  void setExecuted(bool executed);

  QDateTime executionDate() const;
  void setExecutionDate(QDateTime executionDate);

 private:
  QDateTime _executionDate;
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

    RemoteJobDefinition* remoteJobDefinition() const;
    void setRemoteJobDefinition(RemoteJobDefinition* _remote_job_definition);

    JobDefinition *duplicate(QString newName, QString newFileName);

private:
    QString _filename;
    QString _name;
    QString _comment;
    QString _assemblyName;
    QString _assemblyVersion;
    ExecutionDefinition *_executionDefinition;
    RemoteJobDefinition* m_remote_job_definition;
};
}

#endif // JOBDEFINITION_H
