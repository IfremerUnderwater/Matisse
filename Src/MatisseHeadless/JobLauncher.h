#ifndef JOB_LAUNCHER_H
#define JOB_LAUNCHER_H

#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QPair>
#include <QDesktopServices>
#include <QRegExp>

#include <QtDebug>

#include "KeyValueList.h"
#include "dim2_file_reader.h"
#include "MatissePreferences.h"
#include "MatisseEngine.h"
#include "SystemDataManager.h"
#include "ProcessDataManager.h"
#include "platform_comparison_status.h"
#include "string_utils.h"

using namespace MatisseTools;

namespace MatisseCmd {


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
}

#endif // JOB_LAUNCHER_H
