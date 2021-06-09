#include <QStyle>

#include "job_launcher.h"
#include "graphical_charter.h"

namespace matisse {

JobLauncher::JobLauncher(QObject *_parent) : 
      m_engine(_parent, true), // initialize engine as server
      m_current_job(NULL),
      m_current_assembly(NULL),
      m_new_assembly(NULL)
{
  // useless but does not build otherwise -> ugly ..................
  GraphicalCharter &graph_charter = GraphicalCharter::instance();
  int a = graph_charter.dpiScaled(0);
  QString b("");
  FileUtils::createTempDirectory(b);
  QString source;
  QMap<QString, QString> properties;
  StringUtils::substitutePlaceHolders(source, properties);

  // ...............................................................

  m_engine.setJobLauncher(this);
}

JobLauncher::~JobLauncher() {}

void JobLauncher::initServer() {
  m_engine.init();

  connect(&m_engine, SIGNAL(si_jobProcessed(QString, bool)), this,
          SLOT(sl_jobProcessed(QString, bool)));
  connect(&m_engine, SIGNAL(si_userInformation(QString)), this,
          SLOT(sl_userInformation(QString)));
  connect(&m_engine, SIGNAL(si_processCompletion(quint8)), this,
          SLOT(sl_processCompletion(quint8)));
}

void JobLauncher::init() {
  initServer();

  ProcessDataManager* process_data_manager = ProcessDataManager::instance();

  /* Load elements */
  bool load_status = process_data_manager->loadAssembliesAndJobs();
  if (!load_status) {
    qCritical() << tr("Could not initialize job launcher");
    exit(1);
  }
}

void JobLauncher::launchJob(QString _job_name) {
  qDebug() << "Launching job...";

  ProcessDataManager* process_data_manager = ProcessDataManager::instance();

  if (!m_current_job) {
    m_current_job = process_data_manager->getJob(_job_name);

    if (!m_current_job) {
      qCritical() << QString(
                         "Selected job '%1' not found in local repository, "
                         "impossible to launch")
                         .arg(_job_name);
      return;
    }
  } else {
    _job_name = m_current_job->name();
  }

  QString assembly_name = m_current_job->assemblyName();

  AssemblyDefinition *assemblyDef =
      process_data_manager->getAssembly(assembly_name);
  if (!assemblyDef) {
    qCritical() << "Assembly error" << assembly_name;
    return;
  }

  // Copy XML files to result path (crashing because of strong dependance to
  // graphic classes
  // QString resultPath =
  // _server.parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
  //_processDataManager->copyJobFilesToResult(jobName, resultPath);

  emit signal_processRunning();

  bool run_success = m_engine.processJob(*m_current_job);

  if (!run_success) {
    qCritical() << QString("Error launching job");
  }
}

void JobLauncher::sl_processCompletion(quint8 _ret_code) {
  qDebug() << tr("Job completion status: %1").arg(_ret_code);
}

void JobLauncher::sl_jobProcessed(QString _job_name, bool _is_canceled) {
  if (_is_canceled) {
    qDebug() << QString("Job '%1' canceled").arg(_job_name);
    exit(1);
  } else {
    bool error = m_engine.errorFlag();
    if (error) {
      qCritical() << QString("Job '%1' failed").arg(_job_name);
      exit(1);
    }

    qDebug() << QString("Job '%1' executed").arg(_job_name);

    ProcessDataManager* process_data_manager = ProcessDataManager::instance();
    JobDefinition *job_def = process_data_manager->getJob(_job_name);
    QDateTime now = QDateTime::currentDateTime();
    job_def->executionDefinition()->setExecutionDate(now);
    process_data_manager->writeJobFile(job_def, true);
    exit(0);
  }
}

void JobLauncher::sl_userInformation(QString _user_information) {
  qDebug() << "==> User Information : " << _user_information;
}

}  // namespace matisse
