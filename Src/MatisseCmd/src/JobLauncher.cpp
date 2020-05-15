#include <QStyle>

#include "JobLauncher.h"
#include "GraphicalCharter.h"

using namespace MatisseTools;
using namespace MatisseServer;

JobLauncher::JobLauncher(QObject *parent) :

    _currentJob(NULL),
    _currentAssembly(NULL),
    _newAssembly(NULL),
    _systemDataManager(NULL),
    _processDataManager(NULL)
{
    // useless but does not build otherwise -> ugly ..................
//    GraphicalCharter &graph_charter = GraphicalCharter::instance();
//    int a = graph_charter.dpiScaled(0);
//    QString b("");
//    FileUtils::createTempDirectory(b);
//    QString source;
//    QMap<QString,QString> properties;
//    StringUtils::substitutePlaceHolders(source, properties);
//    // ...............................................................

    _server.setMainGui(this);

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
    _server.setSystemDataManager(_systemDataManager);
    _server.setProcessDataManager(_processDataManager);
    _server.init();

    connect(&_server, SIGNAL(signal_jobProcessed(QString, bool)), this, SLOT(slot_jobProcessed(QString, bool)));
    connect(&_server, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)), this, SLOT(slot_jobShowImageOnMainView(QString,Image *)));
    connect(&_server, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(&_server, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
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

    bool runSuccess = _server.processJob(*_currentJob);

    if (!runSuccess) {
       qCritical() << QString("Error launching job");
    }

}
