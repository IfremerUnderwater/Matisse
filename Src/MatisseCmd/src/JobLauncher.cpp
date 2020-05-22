#include <QStyle>

#include "JobLauncher.h"
#include "GraphicalCharter.h"

using namespace MatisseTools;
using namespace MatisseCmd;

JobLauncher::JobLauncher(QObject *parent) :

    _currentJob(NULL),
    _currentAssembly(NULL),
    _newAssembly(NULL),
    _systemDataManager(NULL),
    _processDataManager(NULL)
{
    // useless but does not build otherwise -> ugly ..................
    GraphicalCharter &graph_charter = GraphicalCharter::instance();
    int a = graph_charter.dpiScaled(0);
    QString b("");
    FileUtils::createTempDirectory(b);
    QString source;
    QMap<QString,QString> properties;
    StringUtils::substitutePlaceHolders(source, properties);

    // ...............................................................

    _engine.setJobLauncher(this);

}

JobLauncher::~JobLauncher()
{
}

void JobLauncher::setSystemDataManager(SystemDataManager *systemDataManager)
{
    _systemDataManager = systemDataManager;
}

void JobLauncher::setProcessDataManager(ProcessDataManager *processDataManager)
{
    _processDataManager = processDataManager;
}


void JobLauncher::initServer()
{
    _engine.setSystemDataManager(_systemDataManager);
    _engine.setProcessDataManager(_processDataManager);
    _engine.init();

    connect(&_engine, SIGNAL(signal_jobProcessed(QString, bool)), this, SLOT(slot_jobProcessed(QString, bool)));
    connect(&_engine, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)), this, SLOT(slot_jobShowImageOnMainView(QString,Image *)));
    connect(&_engine, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(&_engine, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
}


void JobLauncher::init()
{

    initServer();

    /* Load elements */
    _processDataManager->loadAssembliesAndJobs();
}

void JobLauncher::launchJob(QString jobName)
{
    qDebug() << "Launching job...";


    if (!_currentJob) {

        _currentJob = _processDataManager->getJob(jobName);

        if (!_currentJob) {
            qCritical() << QString("Selected job '%1' not found in local repository, impossible to launch").arg(jobName);
            return;
        }
    } else {
        jobName = _currentJob->name();
    }

    QString assemblyName = _currentJob->assemblyName();


    AssemblyDefinition * assemblyDef = _processDataManager->getAssembly(assemblyName);
    if (!assemblyDef) {
        qCritical() << "Assembly error" << assemblyName;
        return;
    }

    // Copy XML files to result path (crashing because of strong dependance to graphic classes
    //QString resultPath = _server.parametersManager()->getParameterValue(DATASET_PARAM_OUTPUT_DIR);
    //_processDataManager->copyJobFilesToResult(jobName, resultPath);

    emit signal_processRunning();

    bool runSuccess = _engine.processJob(*_currentJob);

    if (!runSuccess) {
       qCritical() << QString("Error launching job");
    }

}

void JobLauncher::slot_processCompletion(quint8 retCode)
{
    qDebug() << tr("Job completion status: %1").arg(retCode);
}

void JobLauncher::slot_jobProcessed(QString jobName, bool isCanceled)
{
    if (isCanceled) {
        qDebug() << tr("Job '%1' canceled").arg(jobName);
        exit(1);
    } else {
        qDebug() << tr("Job '%1' executed, quitting...").arg(jobName);
        exit(0);
    }
}
