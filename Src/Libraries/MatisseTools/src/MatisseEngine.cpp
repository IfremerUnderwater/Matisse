#include "MatisseEngine.h"

using namespace MatisseTools;

MatisseEngine::MatisseEngine(QObject *_parent, bool _is_server_mode) :
    QObject(_parent),
    m_is_server_mode(_is_server_mode),
    _jobLauncher(NULL),
    _systemDataManager(NULL),
    _processDataManager(NULL),
    _jobServer(NULL),
    _currentJob(NULL),
    _thread(NULL)
{

}


MatisseEngine::~MatisseEngine()
{
    if (_jobServer) {
        delete _jobServer;
    }
    if(_thread)
        _thread->exit();
    if(_currentJob)
        _currentJob->stop(true);
}


QList<Processor*> const MatisseEngine::getAvailableProcessors() {
    return _processors.values();
}

QList<ImageProvider*> const MatisseEngine::getAvailableImageProviders() {
    return _imageProviders.values();
}

const QList<RasterProvider *> MatisseEngine::getAvailableRasterProviders()
{
    return _rasterProviders.values();
}

void MatisseEngine::addParametersForImageProvider(QString name)
{
    qDebug() << "Loading parameters for image provider " << name;

    if (!_imageProviders.contains(name)) {
        qCritical() << QString("Image provider '%1' could not be found.").arg(name);
        return;
    }

    ImageProvider* source = _imageProviders.value(name);
    QList<MatisseParameter> expectedParams = source->expectedParameters();
    _expectedParametersByModule.insert(name, expectedParams);

    foreach(MatisseParameter expectedParam, expectedParams) {
        qDebug() << "Source expects param " << expectedParam.structure << expectedParam.param;
        _dicoParamMgr->addUserModuleForParameter(name, expectedParam.structure, expectedParam.param);
    }

}

void MatisseEngine::addParametersForProcessor(QString name)
{
    qDebug() << "Loading parameters for Processor " << name;

    if (!_processors.contains(name)) {
        qCritical() << QString("Processor '%1' could not be found.").arg(name);
        return;
    }

    Processor* processor = _processors.value(name);
    QList<MatisseParameter> expectedParams = processor->expectedParameters();
    _expectedParametersByModule.insert(name, expectedParams);

    foreach(MatisseParameter expectedParam, expectedParams) {
        qDebug() << "Processor expects param " << expectedParam.structure << expectedParam.param;
        _dicoParamMgr->addUserModuleForParameter(name, expectedParam.structure, expectedParam.param);
    }

}


void MatisseEngine::addParametersForRasterProvider(QString name)
{
    qDebug() << "Loading parameters for raster provider " << name;

    if (!_rasterProviders.contains(name)) {
        qCritical() << QString("Raster provider '%1' could not be found.").arg(name);
        return;
    }

    RasterProvider* destination = _rasterProviders.value(name);
    QList<MatisseParameter> expectedParams = destination->expectedParameters();
    _expectedParametersByModule.insert(name, expectedParams);

    foreach(MatisseParameter expectedParam, expectedParams) {
        qDebug() << "Destination expects param " << expectedParam.structure << expectedParam.param;
        _dicoParamMgr->addUserModuleForParameter(name, expectedParam.structure, expectedParam.param);
    }

}

bool MatisseEngine::removeModuleAndExpectedParameters(QString name)
{
    qDebug() << QString("Remove module '%1' from expected parameters list").arg(name);

    if (!_expectedParametersByModule.contains(name)) {
        qCritical() << "Unknown module" << name;
        return false;
    }

    QList<MatisseParameter> expectedParameters = _expectedParametersByModule.value(name);
    foreach(MatisseParameter param, expectedParameters) {
        _dicoParamMgr->removeUserModuleForParameter(name, param.structure, param.param);
    }

    return true;
}

MatisseParameters* MatisseEngine::buildMatisseParameters(JobDefinition &job) {


    QString file = _processDataManager->getJobParametersFilePath(job.name());

    qDebug() << "Chargement du fichier de paramètres : " << file;
    MatisseParameters* parameters = NULL;
    if (QFile::exists(file)) {
        parameters = new MatisseParameters(file);
    } else {
        setMessageStr(tr("Cannot find parameters file: %1").arg(file));
    }

    return parameters;

}

void MatisseEngine::setMessageStr(QString messageStr, bool error)
{
    _messageStr = messageStr;
    _errorFlag = error;

    if (!_messageStr.isEmpty()) {
        if (_errorFlag) {
            qDebug() << "[ERREUR SERVEUR]:" << _messageStr;
        } else {
            qDebug() << "[MESSAGE SERVEUR]:" << _messageStr;
        }
    } else {
        // si on reinitialise, pas d'erreur...
        _errorFlag = false;
    }
}

void MatisseEngine::slot_currentJobProcessed()
{
    if(_currentJob == NULL)
        return;

    JobDefinition jobDefinition = _currentJob->jobDefinition();
    bool isCancelled=_currentJob->isCancelled();
    QString jobName = jobDefinition.name();
    jobDefinition.executionDefinition()->setExecuted(!isCancelled);
    if (!isCancelled) {
        jobDefinition.executionDefinition()->setResultFileNames(_currentJob->resultFileNames());
        _jobServer->sendExecutionNotification(jobName);
    }

    disconnect(this, SLOT(slot_currentJobProcessed()));
    disconnect(this, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)));
    disconnect(this, SIGNAL(signal_userInformation(QString)));
    disconnect(this, SIGNAL(signal_processCompletion(quint8)));

    _currentJob->deleteLater();
    _currentJob = NULL;
    emit signal_jobProcessed(jobName, isCancelled);
}

bool MatisseEngine::buildJobTask(AssemblyDefinition &assembly, JobDefinition &jobDefinition, MatisseParameters *matisseParameters)
{
    ImageProvider* imageProvider = NULL;
    QList<Processor*> processorList;
    RasterProvider* rasterProvider = NULL;

    qDebug() << "Check assembly";


    // Verifier si les paramètres attendus sont présents pour la source
    QString sourceName = assembly.sourceDefinition()->name();
    qDebug() << "Verification présence de la source" << sourceName;
    imageProvider = _imageProviders.value(sourceName);
    if (!imageProvider) {
        setMessageStr(tr("Source module not found: %1").arg(sourceName));
        return false;
    }

    qDebug() << "Verification des paramètres attendus";
    QList<MatisseParameter> expectedParams = imageProvider->expectedParameters();
    foreach (MatisseParameter mp, expectedParams) {
        if (!matisseParameters->containsParam(mp.structure, mp.param)) {
            setMessageStr(tr("Required parameter not found: (%1, %2) for %3").arg(mp.structure, mp.param, sourceName));
            return false;
        }
    }

    quint32 maxOrder = 0;
    // Verifier si les paramètres attendus sont présents pour les algorithmes
    qDebug() << "Verification présence des processeurs";
    QList<ProcessorDefinition*> processorDefs= assembly.processorDefs();
    foreach (ProcessorDefinition* procDef, processorDefs) {
        quint32 order = procDef->order();
        maxOrder = qMax(maxOrder, order);
        QString processorName = procDef->name();
        qDebug() << "Processeur" << processorName;
        if (order==0) {
            setMessageStr(tr("Processor defined in wrong order: %1").arg(processorName));
            // impossible :source
            return false;
        }
        else {
            Processor *processor = _processors.value(processorName ,NULL);
            if (!processor) {
                setMessageStr(tr("Cannot find processor module: %1").arg(processorName));
                return false;
            }

            expectedParams = processor->expectedParameters();
            foreach (MatisseParameter mp, expectedParams) {
                if (!matisseParameters->containsParam(mp.structure, mp.param)) {
                    setMessageStr(tr("Required parameter not found in assembly: (%1, %2) for %3").arg(mp.structure, mp.param, processorName));
                    return false;
                }
            }
        }
    }


    // Verifier si les paramètres attendus sont présents pour la destination
    qDebug() << "Verification présence destination";
    DestinationDefinition * destinationDef= assembly.destinationDefinition();
    if (!destinationDef) {
         setMessageStr(tr("Destination not defined"));
        return false;
    }
    quint32 order = destinationDef->order();
    QString destinationName = destinationDef->name();
    if (order <= maxOrder) {
        setMessageStr(tr("Destination defined with wrong order: %1").arg(destinationName));
        return false;
    }
    else {
        rasterProvider = _rasterProviders.value(destinationName);
        if (!rasterProvider) {
            setMessageStr(tr("Cannot find destionation module: %1").arg(destinationName));
            return false;
        }
        expectedParams = rasterProvider->expectedParameters();
        foreach (MatisseParameter mp, expectedParams) {
            if (!matisseParameters->containsParam(mp.structure, mp.param)) {
                setMessageStr(tr("Cannot find required parameter for assembly: (%1, %2) for %3").arg(mp.structure, mp.param, destinationName));
                return false;
            }
        }
    }


    qDebug() << "Creation des ImageSet ";
    // Recuperation de l'ordre du raster...
    quint32 destinationOrder = assembly.destinationDefinition()->order();

    // Ports par ordre
    QHash<quint32, QList<ImageSetPort *>* > inProcessorPortsByOrder;
    QHash<quint32, QList<ImageSetPort *>* > outProcessorPortsByOrder;


    QList<ConnectionDefinition*> connectionDefs = assembly.connectionDefs();
    foreach (ConnectionDefinition* conDef, connectionDefs) {
        quint32 startOrder = conDef->startOrder();
        quint32 endOrder = conDef->endOrder();
        QList<ImageSetPort *> *outProcessorPorts = outProcessorPortsByOrder.value(startOrder, NULL);
        if (!outProcessorPorts) {
            outProcessorPorts = new QList<ImageSetPort *>;
            outProcessorPortsByOrder.insert(startOrder, outProcessorPorts);
        }
        QList<ImageSetPort *> *inProcessorPorts = inProcessorPortsByOrder.value(endOrder, NULL);
        if (!inProcessorPorts) {
            inProcessorPorts = new QList<ImageSetPort *>;
            inProcessorPortsByOrder.insert(endOrder, inProcessorPorts);
        }
        // out source
        ImageSetPort *outPort = new ImageSetPort();
        ImageSetPort *inPort = new ImageSetPort();
        quint32 outPortNumber = conDef->startLine();
        quint32 inPortNumber = conDef->endLine();
        outPort->portNumber = outPortNumber;
        inPort->portNumber = inPortNumber;
        if (startOrder==0) {
            outPort->imageSet = imageProvider->imageSet(outPortNumber);
        } else {
            qDebug() << "  Creation de out port processor ";
            // Creation d'un imageset vide
            outPort->imageSet = new ImageSet();

        }

        inPort->imageSet=outPort->imageSet;


        outProcessorPorts->append(outPort);
        inProcessorPorts->append(inPort);
    }



    qDebug() << "Configuration des Processeurs";
    foreach (ProcessorDefinition* procDef, processorDefs) {
        quint32 order = procDef->order();
        QString processorName = procDef->name();
        if (order==0) {
            // impossible :source
        } else if (order == destinationOrder) {
            // impossible : destination
        }
        else {
            Processor *processor = _processors.value(processorName);

            qDebug() << "Fin assemblage du processeur " << processorName;
            QList<ImageSetPort *> * inPorts =  inProcessorPortsByOrder.value(order);
            if (inPorts) {
                qDebug() << "  Ajout des connections entrantes de " << processorName << " - nb: " << inPorts->size() ;
                processor->setInputPortList(inPorts);
                foreach (ImageSetPort* inPort, *inPorts)
                {
                    qDebug() << "  Port entrant " << inPort->portNumber;
                    inPort->imageSet->setOutPort(inPort);
                    inPort->imageListener = processor;
                }
            }
            QList<ImageSetPort *> * outPorts =  outProcessorPortsByOrder.value(order);
            if (outPorts) {
                qDebug() << "  Ajout des connections sortantes de " << processorName << " - nb: " << outPorts->size() ;
                processor->setOutputPortList(outPorts);
            }
            processorList.append(processor);
        }
    }

    qDebug() << "Configuration du RasterProvider";

    order = destinationDef->order();
    QList<ImageSetPort *> * inPorts =  inProcessorPortsByOrder.value(order);
    if (inPorts && inPorts->size()==1) {
        ImageSetPort* inPort = inPorts->at(0);
        qDebug() << "Raster  Port entrant " << inPort->portNumber;
        rasterProvider->setImageSet(inPort->imageSet);
        inPort->imageSet->setOutPort(inPort);
        inPort->imageListener = rasterProvider;
    }


    _currentJob = new JobTask(imageProvider, processorList, rasterProvider, jobDefinition, matisseParameters);
     _currentJob->setIsServerMode(m_is_server_mode);
    _currentJob->setJobLauncher(_jobLauncher);

    return true;

}



bool MatisseEngine::processJob(JobDefinition &jobDefinition)
{
    setMessageStr();

    if (_currentJob) {
        qDebug() << "A Thread is running";
        // TODO Queue jobs?
        return false;
    }

     setMessageStr();

    // Deleted by JobTask
    MatisseParameters* parameters = buildMatisseParameters(jobDefinition);



    if (!parameters) {
        setMessageStr(tr("Invalid parameters file"));
        return false;
    }

    qDebug() << "Dump parametres:" << parameters->dumpStructures();

    QString assemblyName = jobDefinition.assemblyName();
    AssemblyDefinition * assemblyDefinition = _processDataManager->getAssembly(assemblyName);

    if (!assemblyDefinition) {
        setMessageStr(tr("Cannot load assembly %1").arg(assemblyName));
        return false;
    }

    if (!buildJobTask(*assemblyDefinition, jobDefinition, parameters)) {
        setMessageStr(tr("Running assembly failed"));
        return false;
    }

    QThread::currentThread()->setObjectName("GUI");

    // TODO : penser a deleter le pointeur
    _thread = new QThread;
    _thread->setObjectName("JobTask");
    connect(_thread, SIGNAL(started()), _currentJob, SLOT(slot_start()));
    connect(_thread, SIGNAL(finished()), _currentJob, SLOT(slot_stop()));
    connect(_currentJob, SIGNAL(signal_jobStopped()), this, SLOT(slot_currentJobProcessed()));
    connect(_currentJob, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)), this, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)));
    connect(_currentJob, SIGNAL(signal_userInformation(QString)), this, SIGNAL(signal_userInformation(QString)));
    connect(_currentJob, SIGNAL(signal_processCompletion(quint8)), this, SIGNAL(signal_processCompletion(quint8)));
    _currentJob->moveToThread(_thread);
    qDebug() << "Démarrage de la tache";
    _thread->start();

    return true;

}

bool MatisseEngine::isProcessingJob()
{
    return _currentJob!=NULL;
}

bool MatisseEngine::stopJob(bool cancel)
{
    if (_currentJob) {
        _currentJob->stop(cancel);

        disconnect(this, SLOT(slot_currentJobProcessed()));
        disconnect(this, SIGNAL(signal_jobShowImageOnMainView(QString,Image *)));
        disconnect(this, SIGNAL(signal_userInformation(QString)));
        disconnect(this, SIGNAL(signal_processCompletion(quint8)));

        qDebug() << "Fin du Thread" ;
        _thread->quit();
    }
    return true;
}

bool MatisseEngine::errorFlag()
{
    return _errorFlag;
}

QString MatisseEngine::messageStr()
{
    return _messageStr;
}

#ifdef WIN32
#define SHARED_DLL_EXT "*.dll"
#else
#define SHARED_DLL_EXT "*.so"
#endif

void MatisseEngine::init(){

    _jobServer = new JobServer(_systemDataManager->port(), _processDataManager);

    loadParametersDictionnary();

    // Load processors
    QDir processorsDir = QDir(_systemDataManager->getDllPath() + QDir::separator() +  "processors");
    setMessageStr();



    foreach (QString fileName, processorsDir.entryList(QStringList() << SHARED_DLL_EXT , QDir::Files)) {
        qDebug() <<"Loading Processor DLL " << fileName;
        QPluginLoader loader(processorsDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
            Processor* newInstance = qobject_cast<Processor *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "Processor DLL " << newInstance->name() << " loaded.";
            _processors.insert(newInstance->name(), newInstance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << fileName;
            if(loader.isLoaded())
            {
                QObject *pluginObject = loader.instance();
                if(pluginObject)
                {
                    Processor* newInstance = qobject_cast<Processor *>(pluginObject); // On réinterprète alors notre QObject
                    qDebug() << "Processor DLL " << newInstance->name() << " loaded.";
                    _processors.insert(newInstance->name(), newInstance);

                }
                else
                {
                    QString problem = loader.errorString();
                    qDebug() <<  "Plugin load problem retry: " << problem;
                }
            }
        }

    }

    // Load imageProviders
    QDir imageProvidersDir = QDir(_systemDataManager->getDllPath() + QDir::separator() + "imageProviders");
    foreach (QString fileName, imageProvidersDir.entryList(QStringList() << SHARED_DLL_EXT, QDir::Files)) {
        qDebug() <<"Loading ImageProvider DLL " << fileName;
        QPluginLoader loader(imageProvidersDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            ImageProvider* newInstance = qobject_cast<ImageProvider *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "ImageProvider DLL " << newInstance->name() << " loaded.";
            _imageProviders.insert(newInstance->name(), newInstance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << fileName;
        }

    }

    // Load rasterProviders
    QDir rasterProvidersDir = QDir(_systemDataManager->getDllPath() + QDir::separator() + "rasterProviders");

    foreach (QString fileName, rasterProvidersDir.entryList(QStringList() << SHARED_DLL_EXT, QDir::Files)) {
        qDebug() <<"Loading RasterProvider DLL " << fileName;
        QPluginLoader loader(rasterProvidersDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            RasterProvider* newInstance = qobject_cast<RasterProvider *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "RasterProvider DLL " << newInstance->name() << " loaded.";
            _rasterProviders.insert(newInstance->name(), newInstance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << fileName;
        }

    }

}


JobTask::JobTask(ImageProvider* imageProvider, QList<Processor*> processors, RasterProvider* rasterProvider,
                 JobDefinition &jobDefinition, MatisseParameters *parameters )
    : m_user_log_file(NULL),
      _imageProvider(imageProvider),
      _processors(processors),
      _rasterProvider(rasterProvider),
      _jobDefinition(jobDefinition),
      _matParameters(parameters),
      _isCancelled(false)
{
    // Create the log file in the output folder
    QString dataset_dir = _matParameters->getStringParamValue("dataset_param", "dataset_dir");

    QDir output_dir(_matParameters->getStringParamValue("dataset_param", "output_dir"));
    QDir absolute_out_dir;

    if (output_dir.isRelative())
    {
        absolute_out_dir.setPath(dataset_dir + QDir::separator() + output_dir.path() + QDir::separator() + "log");
    }
    else
        absolute_out_dir.setPath(output_dir.path() + QDir::separator() + "log");

    if (!absolute_out_dir.exists())
    {
        absolute_out_dir.mkpath(absolute_out_dir.path());
    }

    m_user_log_file = new QFile(absolute_out_dir.absoluteFilePath("user_log.txt"));

    if (m_user_log_file->open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_log_file_opened = true;
    }
}

JobTask::~JobTask()
{
    delete _matParameters;
}

void JobTask::stop(bool cancel)
{
    _isCancelled = cancel;

    _imageProvider->askToStop(cancel);

    foreach (Processor *processor, _processors) {
        processor->askToStop(cancel);
    }

    _rasterProvider->askToStop(cancel);
}

void JobTask::slot_start()
{

    // TODO Ask for external creation of context!
    bool ok;
    _context = new Context;

    connect(_imageProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(_imageProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    if (!m_is_server_mode) {
      connect(_imageProvider, SIGNAL(signal_showInformationMessage(QString, QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString, QString)));
      connect(_imageProvider, SIGNAL(signal_showErrorMessage(QString, QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString, QString)));
      connect(_imageProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
      connect(_imageProvider, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
      connect(_imageProvider, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
      connect(_imageProvider, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
    }

    ok = _imageProvider->callConfigure(_context, _matParameters);
    if (!ok) {
      /* Bad parameters or dataset not found */
      qCritical() << "Error on raster provider configuration";
      if (m_is_server_mode) { // for non-regression with client mode (is there a reason why the task should continue in that case ?) 
          slot_fatalError();
      }
      return;
    }


    qDebug() << "Configuration des Processeurs";
    foreach (Processor* processor, _processors) {
        qDebug() << "Configuration du processeur " << processor->name();
        connect(processor, SIGNAL(signal_showImageOnMainView(Image*)), this, SLOT(slot_showImageOnMainView(Image*)));
        connect(processor, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
        connect(processor, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
        connect(processor, SIGNAL(signal_fatalError()), this, SLOT(slot_fatalError()));
        connect(processor, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
        if (!m_is_server_mode) {
          connect(processor, SIGNAL(signal_showInformationMessage(QString,QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString,QString)));
          connect(processor, SIGNAL(signal_showErrorMessage(QString,QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString,QString)));
          connect(processor, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
          connect(processor, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
          connect(processor, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
        }
        processor->callConfigure(_context, _matParameters);
    }

    qDebug() << "Configuration de la destination";
    connect(_rasterProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(_rasterProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    connect(_rasterProvider, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
    if (!m_is_server_mode) {
      connect(_rasterProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString,QString)));
      connect(_rasterProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString,QString)));
      connect(_rasterProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
      connect(_rasterProvider, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
      connect(_rasterProvider, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
    }
    ok = _rasterProvider->callConfigure(_context, _matParameters);
    if (!ok) {
        qDebug() << "Error on raster provider configuration";
        return;
    }


    qDebug() << "Démarrage du raster provider";
    ok = _rasterProvider->callStart();
    if (!ok) {
        qDebug() << "Error on raster provider start";
        return;
    }

    qDebug() << "Démarrage des processeurs";
    foreach (Processor *processor, _processors) {
        ok = processor->callStart();
        if (!ok) {
            qDebug() << "Error on processor start: " << processor->name();
            return;
        }
    }

    qDebug() << "Démarrage du image provider";
    ok = _imageProvider->callStart();
    if (!ok) {
        qDebug() << "Error on image provider start";
        return;
    }

    qDebug() << "Fin de slot_start";

    bool isRealTime = _imageProvider->isRealTime();
    if (!isRealTime) {
        QThread::currentThread()->exit();
    }

}

void JobTask::slot_stop()
{

    qDebug() << "Arret du image provider";
    _imageProvider->callStop();

    qDebug() << "Arret des processeurs";
    foreach (Processor *processor, _processors) {
        processor->callStop();
    }
    disconnect(this, SLOT(slot_showImageOnMainView(Image*)));

    qDebug() << "Arret du raster provider";
    _rasterProvider->callStop();

    /* Disconnecting user information signals */
    disconnect(this, SLOT(slot_userInformation(QString)));
    disconnect(this, SLOT(slot_processCompletion(quint8)));

    _resultFileNames.clear();
    if (!_isCancelled) {
        // recuperation du nom du fichier de sortie

        foreach (QFileInfo rasterInfo, _rasterProvider->rastersInfo()) {
            _resultFileNames << rasterInfo.absoluteFilePath();
        }
    }

    // Disconnect everything
    disconnect(_imageProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    disconnect(_imageProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    disconnect(_imageProvider, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
    if (!m_is_server_mode) {
      disconnect(_imageProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString,QString)));
      disconnect(_imageProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString,QString)));
      disconnect(_imageProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
      disconnect(_imageProvider, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
      disconnect(_imageProvider, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
    }

    foreach (Processor* processor, _processors) {
        if(!processor->okStatus())
            _isCancelled = true;
        disconnect(processor, SIGNAL(signal_showImageOnMainView(Image*)), this, SLOT(slot_showImageOnMainView(Image*)));
        disconnect(processor, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
        disconnect(processor, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
        disconnect(processor, SIGNAL(signal_fatalError()), this, SLOT(slot_fatalError()));
        disconnect(processor, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
        if (!m_is_server_mode) {
          disconnect(processor, SIGNAL(signal_showInformationMessage(QString,QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString,QString)));
          disconnect(processor, SIGNAL(signal_showErrorMessage(QString,QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString,QString)));
          disconnect(processor, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
          disconnect(processor, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
          disconnect(processor, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
        }
    }

    disconnect(_rasterProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    disconnect(_rasterProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    disconnect(_rasterProvider, SIGNAL(signal_addToLog(QString)), this, SLOT(slot_logToFile(QString)));
    if (!m_is_server_mode) {
      disconnect(_rasterProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _jobLauncher, SLOT(slot_showInformationMessage(QString,QString)));
      disconnect(_rasterProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _jobLauncher, SLOT(slot_showErrorMessage(QString,QString)));
      disconnect(_rasterProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _jobLauncher, SLOT(slot_show3DFileOnMainView(QString)));
      disconnect(_rasterProvider, SIGNAL(signal_addRasterFileToMap(QString)), _jobLauncher, SLOT(slot_addRasterFileToMap(QString)));
      disconnect(_rasterProvider, SIGNAL(signal_addToLog(QString)), _jobLauncher, SLOT(slot_addToLog(QString)));
    }

    if(_context != NULL)
    {
        delete _context;
        _context = NULL;
        emit signal_jobStopped();
    }

    if (m_user_log_file)
    {
        m_user_log_file->close();
        delete m_user_log_file;
        m_user_log_file = NULL;
    }

}

bool JobTask::isCancelled() const
{
    return _isCancelled;
}

void JobTask::slot_showImageOnMainView(Image *image)
{
    emit signal_jobShowImageOnMainView(_jobDefinition.name(), image);
}

void JobTask::slot_userInformation(QString userText)
{
    emit signal_userInformation(userText);
}

void JobTask::slot_processCompletion(quint8 percentComplete)
{
    emit signal_processCompletion(percentComplete);
}

void JobTask::slot_fatalError()
{
    _isCancelled = true;
    stop(true);
    _isCancelled = true;
    emit signal_jobStopped();
    slot_stop();
}

void JobTask::slot_logToFile(QString _logInfo)
{
    QTextStream out_stream(m_user_log_file);
    out_stream << _logInfo << "\n";
}

void JobTask::setJobLauncher(QObject *jobLauncher)
{
    _jobLauncher = jobLauncher;
}

void JobTask::setIsServerMode(bool _is_server_mode) 
{
  m_is_server_mode = _is_server_mode;
}

QStringList JobTask::resultFileNames() const
{
    return _resultFileNames;
}

JobDefinition &JobTask::jobDefinition() const
{
    return _jobDefinition;
}


bool MatisseEngine::loadParametersDictionnary()
{
    QXmlSchema dictionnarySchema;

    qDebug() << "Loading MatisseParametersDictionnary.xsd schema...";

    QString dico_schema_path = _systemDataManager->getBinRootDir() +
                    QDir::separator() + "schemas" + QDir::separator() +
                    "MatisseParametersDictionnary.xsd";

    //QFile dicoXsdFile("schemas/MatisseParametersDictionnary.xsd");
    QFile dicoXsdFile(dico_schema_path);

    if (!dicoXsdFile.exists()) {
        qFatal("%s\n",QString("Error finding ").append(dicoXsdFile.fileName()).toStdString().c_str());
    }

    if (!dicoXsdFile.open(QIODevice::ReadOnly)) {
        qFatal("%s\n","Error opening MatisseParametersDictionnary.xsd");
    }

    if (!dictionnarySchema.load(&dicoXsdFile, QUrl::fromLocalFile(dicoXsdFile.fileName()))) {
        qFatal("%s\n","Error loading ParametersDictionnary.xsd");
    }

    if (!dictionnarySchema.isValid()) {
        qFatal("%s\n","Error ParametersDictionnary.xsd is not valid");
    }

    qDebug() << "MatisseParametersDictionnary.xsd is a valid schema";
    dicoXsdFile.close();

    qDebug() << "Loading dictionnary file...";

    QString dico_path = _systemDataManager->getBinRootDir() +
                        QDir::separator() + "config" + QDir::separator() +
                        "MatisseParametersDictionnary.xml";
    //QFile dicoXmlFile("config/MatisseParametersDictionnary.xml");
    QFile dicoXmlFile(dico_path);

    if (!dicoXmlFile.exists()) {
        qFatal("%s\n",QString("Error finding").append(dicoXmlFile.fileName()).toStdString().c_str());
    }

    if (!dicoXmlFile.open(QIODevice::ReadOnly)) {
        qFatal("%s\n","Error opening MatisseParametersDictionnary.xml");
    }

    QXmlSchemaValidator validator(dictionnarySchema);
    if (!validator.validate(&dicoXmlFile, QUrl::fromLocalFile(dicoXmlFile.fileName()))) {
        qFatal("%s\n","Dictionnary XML file does not conform to schema");
    }

    qDebug() << "XML dictionnary file is consistent with schema";

    _dicoParamMgr = new MatisseParametersManager();
    _dicoParamMgr->readDictionnaryFile("config/MatisseParametersDictionnary.xml");

    return true;
}
void MatisseEngine::setProcessDataManager(ProcessDataManager *processDataManager)
{
    _processDataManager = processDataManager;
}

void MatisseEngine::setSystemDataManager(SystemDataManager *systemDataManager)
{
    _systemDataManager = systemDataManager;
}


void MatisseEngine::setJobLauncher(QObject *jobLauncher)
{
    _jobLauncher = jobLauncher;
}
