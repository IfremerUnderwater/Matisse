#include "Server.h"
#include "AssemblyDefinition.h"
#include "AssemblyGui.h"

using namespace MatisseServer;

Server::Server(QObject *parent) :
    QObject(parent),
    _systemDataManager(NULL),
    _processDataManager(NULL),
    _jobServer(NULL),
    _currentJob(NULL),
    _mainGui(NULL),
    _thread(NULL)
{

}

Server::~Server()
{
    if (_jobServer) {
        delete _jobServer;
    }
    if(_thread)
        _thread->exit();
    if(_currentJob)
        _currentJob->stop(true);
}

//bool Server::setSettingsFile(QString settings)
//{
//    QString standardFile = "config/MatisseSettings.xml";
//    setMessageStr();
//    // lecture du fichier xml de settings
//    if (settings == "") {
//        settings = standardFile;
//    }
//    QFileInfo settingsFile(settings);
//    if (!settingsFile.exists()) {
//        //QMessageBox::critical(this, "Fichier de configuration introuvable", "Impossible de trouver le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier valide en paramètre\nou un fichier " + standardFile + " valide!");
//        setMessageStr(tr("Fichier de configuration introuvable: %1").arg(settingsFile.absoluteFilePath()));
//        return false;
//    }

//    if (!settingsFile.isReadable()) {
//        //QMessageBox::critical(this, "Fichier de configuration illisible", "Impossible de lire le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier lisible en paramètre\nou rendez le fichier " + standardFile + " lisible!");
//        setMessageStr(tr("Fichier de configuration illisible: %1").arg(settingsFile.absoluteFilePath()));
//        return false;
//    }

//    _xmlTool.readMatisseGuiSettings(settings);
//    if (( _xmlTool.getBasePath()) == "") {
//        //QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de XmlRootDir ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
//        setMessageStr(tr("XmlRootDir introuvable dans le fichier de configuration: %1").arg(settingsFile.absoluteFilePath()));
//        return false;
//    }

//    if (( _xmlTool.getDllPath()) == "") {
//        setMessageStr(tr("DllRootDir introuvable dans le fichier de configuration: %1").arg(settingsFile.absoluteFilePath()));
//        return false;
//    }


//    _jobServer = new JobServer(_xmlTool.port(), &_xmlTool);

//    return true;
//}

QList<Processor*> const Server::getAvailableProcessors() {
    return _processors.values();
}

QList<ImageProvider*> const Server::getAvailableImageProviders() {
    return _imageProviders.values();
}

const QList<RasterProvider *> Server::getAvailableRasterProviders()
{
    return _rasterProviders.values();
}

void Server::addParametersForImageProvider(QString name)
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

void Server::addParametersForProcessor(QString name)
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

void Server::addParametersForRasterProvider(QString name)
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

bool Server::removeModuleAndExpectedParameters(QString name)
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

MatisseParameters* Server::buildMatisseParameters(JobDefinition &job) {


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

void Server::setMessageStr(QString messageStr, bool error)
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

//Xml& Server::xmlTool()
//{
//    return _xmlTool;
//}

void Server::slot_currentJobProcessed()
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

bool Server::buildJobTask(AssemblyDefinition &assembly, JobDefinition &jobDefinition, MatisseParameters *matisseParameters)
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
        setMessageStr(tr("Source modul not found: %1").arg(sourceName));
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
    _currentJob->setMainGui(_mainGui);

    return true;

}



bool Server::processJob(JobDefinition &jobDefinition)
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

bool Server::isProcessingJob()
{
    return _currentJob!=NULL;
}

bool Server::stopJob(bool cancel)
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

bool Server::errorFlag()
{
    return _errorFlag;
}

QString Server::messageStr()
{
    return _messageStr;
}

#ifdef WIN32
#define SHARED_DLL_EXT "*.dll"
#else
#define SHARED_DLL_EXT "*.so"
#endif

void Server::init(){

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
    : _imageProvider(imageProvider),
      _processors(processors),
      _rasterProvider(rasterProvider),
      _jobDefinition(jobDefinition),
      _matParameters(parameters),
      _isCancelled(false)
{
}

JobTask::~JobTask()
{
    delete _matParameters;
}

void JobTask::stop(bool cancel)
{
    _isCancelled = cancel;

    qDebug() << "Demande d'arret du image provider";
    _imageProvider->askToStop(cancel);

    qDebug() << "Demande d'arret des processeurs";
    foreach (Processor *processor, _processors) {
        processor->askToStop(cancel);
    }

    qDebug() << "Demande d'arret du raster provider";
    _rasterProvider->askToStop(cancel);
}

void JobTask::slot_start()
{
    // TODO Ask for external creation of context!
    bool ok;
    _context = new Context;

    qDebug() << "Configuration de la source";
    connect(_imageProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(_imageProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    connect(_imageProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
    connect(_imageProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
    connect(_imageProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
    connect(_imageProvider, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
    connect(_imageProvider, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
    connect(_imageProvider, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
//    connect(_imageProvider, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));

    ok = _imageProvider->callConfigure(_context, _matParameters);
    if (!ok) {
        qDebug() << "Error on raster provider configuration";
        return;
    }


    qDebug() << "Configuration des Processeurs";
    foreach (Processor* processor, _processors) {
        qDebug() << "Configuration du processeur " << processor->name();
        connect(processor, SIGNAL(signal_showImageOnMainView(Image*)), this, SLOT(slot_showImageOnMainView(Image*)));
        connect(processor, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
        connect(processor, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
        connect(processor, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
        connect(processor, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
        connect(processor, SIGNAL(signal_fatalError()), this, SLOT(slot_fatalError()));
        connect(processor, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
        connect(processor, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
        connect(processor, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
        connect(processor, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
//        connect(processor, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));
        processor->callConfigure(_context, _matParameters);
    }

    qDebug() << "Configuration de la destination";
    connect(_rasterProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    connect(_rasterProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    connect(_rasterProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
    connect(_rasterProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
    connect(_rasterProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
    connect(_rasterProvider, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
    connect(_rasterProvider, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
    connect(_rasterProvider, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
//    connect(_rasterProvider, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));
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

    // déconnecter tout
    disconnect(_imageProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    disconnect(_imageProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    disconnect(_imageProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
    disconnect(_imageProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
    disconnect(_imageProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
    disconnect(_imageProvider, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
    disconnect(_imageProvider, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
    disconnect(_imageProvider, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
 //   disconnect(_imageProvider, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));

    foreach (Processor* processor, _processors) {
        if(!processor->okStatus())
            _isCancelled = true;
        disconnect(processor, SIGNAL(signal_showImageOnMainView(Image*)), this, SLOT(slot_showImageOnMainView(Image*)));
        disconnect(processor, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
        disconnect(processor, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
        disconnect(processor, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
        disconnect(processor, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
        disconnect(processor, SIGNAL(signal_fatalError()), this, SLOT(slot_fatalError()));
        disconnect(processor, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
        disconnect(processor, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
        disconnect(processor, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
        disconnect(processor, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
//        disconnect(processor, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));
    }

    disconnect(_rasterProvider, SIGNAL(signal_userInformation(QString)), this, SLOT(slot_userInformation(QString)));
    disconnect(_rasterProvider, SIGNAL(signal_processCompletion(quint8)), this, SLOT(slot_processCompletion(quint8)));
    disconnect(_rasterProvider, SIGNAL(signal_showInformationMessage(QString,QString)), _mainGui, SLOT(slot_showInformationMessage(QString,QString)));
    disconnect(_rasterProvider, SIGNAL(signal_showErrorMessage(QString,QString)), _mainGui, SLOT(slot_showErrorMessage(QString,QString)));
    disconnect(_rasterProvider, SIGNAL(signal_show3DFileOnMainView(QString)), _mainGui, SLOT(slot_show3DFileOnMainView(QString)));
    disconnect(_rasterProvider, SIGNAL(signal_addRasterFileToMap(QString)), _mainGui, SLOT(slot_addRasterFileToMap(QString)));
    disconnect(_rasterProvider, SIGNAL(signal_addPolygonToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolygonToMap(basicproc::Polygon,QString,QString)));
    disconnect(_rasterProvider, SIGNAL(signal_addPolylineToMap(basicproc::Polygon,QString,QString)), _mainGui, SLOT(slot_addPolylineToMap(basicproc::Polygon,QString,QString)));
//    disconnect(_rasterProvider, SIGNAL(signal_addQGisPointsToMap(QList<QgsPoint>,QString,QString)), _mainGui, SLOT(slot_addQGisPointsToMap(QList<QgsPoint>,QString,QString)));

    if(_context != NULL)
    {
        delete _context;
        _context = NULL;
        emit signal_jobStopped();
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

void JobTask::setMainGui(AssemblyGui *mainGui)
{
    _mainGui = mainGui;
}

QStringList JobTask::resultFileNames() const
{
    return _resultFileNames;
}

JobDefinition &JobTask::jobDefinition() const
{
    return _jobDefinition;
}


bool Server::loadParametersDictionnary()
{
    QXmlSchema dictionnarySchema;

    qDebug() << "Loading MatisseParametersDictionnary.xsd schema...";

    QFile dicoXsdFile("schemas/MatisseParametersDictionnary.xsd");

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

    QFile dicoXmlFile("config/MatisseParametersDictionnary.xml");

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
void Server::setProcessDataManager(ProcessDataManager *processDataManager)
{
    _processDataManager = processDataManager;
}

void Server::setSystemDataManager(SystemDataManager *systemDataManager)
{
    _systemDataManager = systemDataManager;
}


void Server::setMainGui(AssemblyGui *mainGui_p)
{
    _mainGui = mainGui_p;
}
