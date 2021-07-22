#ifndef MATISSE_JOB_HELPER_H_
#define MATISSE_JOB_HELPER_H_

#include <QObject>
#include <QTreeWidgetItem>
#include "assembly_definition.h"
#include "job_commons.h"
#include "job_definition.h"
#include "matisse_engine.h"

namespace matisse {

class JobHelper : public QObject
{
    Q_OBJECT
public:
    explicit JobHelper(QObject *_parent = nullptr);

    bool archiveJob(JobDefinition *_job);
    bool restoreJobs(AssemblyDefinition *_assembly);
    bool duplicateJob(JobDefinition *_job);
    JobDefinition *createNewJob(QString _assembly_name);
    bool saveJob(JobDefinition *_job);
    bool deleteJob(QString job_name);
    bool prepareJobForExecution(JobDefinition *_job, QString & _status_msg);
    bool jobProcessed(QString _job_name);
    QString archivePath();
    void setEngine(MatisseEngine *_engine) { m_engine = _engine; }
    void setPreferences(MatissePreferences *_preferences) { m_preferences = _preferences; }
    void setIconFactory(MatisseIconFactory *_icon_factory) { m_icon_factory = _icon_factory; }


signals:

private:
    void checkArchiveDirCreated();

    QWidget *m_parent_widget;
    QString m_archive_path;
    MatisseEngine *m_engine;
    MatissePreferences *m_preferences;
    MatisseIconFactory *m_icon_factory;
};

} // namespace matisse

#endif // MATISSE_JOB_HELPER_H_
