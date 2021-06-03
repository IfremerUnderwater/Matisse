#ifndef MATISSE_JOB_LAUNCHER_H_
#define MATISSE_JOB_LAUNCHER_H_

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QPair>
#include <QDesktopServices>
#include <QRegExp>

#include <QtDebug>

#include "key_value_list.h"
#include "dim2_file_reader.h"
#include "matisse_preferences.h"
#include "matisse_engine.h"
#include "system_data_manager.h"
#include "process_data_manager.h"
#include "platform_comparison_status.h"
#include "string_utils.h"

namespace matisse {


class JobLauncher : public QObject
{
    Q_OBJECT
    
public:
    explicit JobLauncher(QObject *_parent = 0);
    ~JobLauncher();

    void init();
    void launchJob(QString _job_name);

private:
    MatisseEngine m_engine;

    AssemblyDefinition *m_new_assembly;
    AssemblyDefinition *m_current_assembly;
    JobDefinition *m_current_job;



private:
    void initServer();

signals:
    void signal_processRunning();
    void signal_processStopped();
    void signal_processFrozen();

public slots:
    void sl_jobProcessed(QString _job_name, bool _is_canceled);
    void sl_processCompletion(quint8 _ret_code);
    void sl_userInformation(QString _user_information);
};

} // namespace matisse

#endif // MATISSE_JOB_LAUNCHER_H_
