#include <QStyle>

#include "JobLauncher.h"
#include "GraphicalCharter.h"

using namespace MatisseTools;

namespace MatisseCmd {

JobLauncher::JobLauncher(QObject *_parent) : 
      m_engine(_parent, true), // initialize engine as server
      m_current_job(NULL),
      m_current_assembly(NULL),
      m_new_assembly(NULL),
      m_system_data_manager(NULL),
      m_process_data_manager(NULL) {
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

void JobLauncher::setSystemDataManager(
    SystemDataManager *_system_data_manager) {
  m_system_data_manager = _system_data_manager;
}

void JobLauncher::setProcessDataManager(
    ProcessDataManager *process_data_manager) {
  m_process_data_manager = process_data_manager;
}

void JobLauncher::initServer() {
  m_engine.setSystemDataManager(m_system_data_manager);
  m_engine.setProcessDataManager(m_process_data_manager);
  m_engine.init();

  connect(&m_engine, SIGNAL(signal_jobProcessed(QString, bool)), this,
          SLOT(sl_jobProcessed(QString, bool)));
  connect(&m_engine, SIGNAL(signal_userInformation(QString)), this,
          SLOT(sl_userInformation(QString)));
  connect(&m_engine, SIGNAL(signal_processCompletion(quint8)), this,
          SLOT(sl_processCompletion(quint8)));
}

void JobLauncher::init() {
  initServer();

  /* Load elements */
  bool load_status = m_process_data_manager->loadAssembliesAndJobs();
  if (!load_status) {
    qCritical() << tr("Could not initialize job launcher");
    exit(1);
  }
}

void JobLauncher::launchJob(QString _job_name) {
  qDebug() << "Launching job...";

  if (!m_current_job) {
    m_current_job = m_process_data_manager->getJob(_job_name);

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
      m_process_data_manager->getAssembly(assembly_name);
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
    qDebug() << tr("Job '%1' canceled").arg(_job_name);
    exit(1);
  } else {
    qDebug() << tr("Job '%1' executed, quitting...").arg(_job_name);
    exit(0);
  }
}

void JobLauncher::sl_userInformation(QString _user_information) {
  qDebug() << "==> User Information : " << _user_information;
}

}  // namespace MatisseCmd