#include "Server.h"
#include "AssemblyDefinition.h"

using namespace MatisseServer;

// TODO Initialiser _JobServer avec une variable des settings
Server::Server(QObject *parent) :
    QObject(parent),
    _xmlTool(),
    _jobServer(NULL),
    _currentJob(NULL)
{

}

Server::~Server()
{
    if (_jobServer) {
        delete _jobServer;
    }
}

bool Server::setSettingsFile(QString settings)
{
    QString standardFile = "./MatisseSettings.xml";
    setMessageStr();
    // lecture du fichier xml de settings
    if (settings == "") {
        settings = standardFile;
    }
    QFileInfo settingsFile(settings);
    if (!settingsFile.exists()) {
        //QMessageBox::critical(this, "Fichier de configuration introuvable", "Impossible de trouver le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier valide en paramètre\nou un fichier " + standardFile + " valide!");
        setMessageStr(tr("Fichier de configuration introuvable: %1").arg(settingsFile.absoluteFilePath()));
        return false;
    }

    if (!settingsFile.isReadable()) {
        //QMessageBox::critical(this, "Fichier de configuration illisible", "Impossible de lire le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier lisible en paramètre\nou rendez le fichier " + standardFile + " lisible!");
        setMessageStr(tr("Fichier de configuration illisible: %1").arg(settingsFile.absoluteFilePath()));
        return false;
    }

    _xmlTool.readMatisseGuiSettings(settings);
    if (( _xmlTool.getBasePath()) == "") {
        //QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de XmlRootDir ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
        setMessageStr(tr("XmlRootDir introuvable dans le fichier de configuration: %1").arg(settingsFile.absoluteFilePath()));
        return false;
    }

    if (( _xmlTool.getDllPath()) == "") {
        setMessageStr(tr("DllRootDir introuvable dans le fichier de configuration: %1").arg(settingsFile.absoluteFilePath()));
        return false;
    }


    _jobServer = new JobServer(_xmlTool.port(), &_xmlTool);

    return true;
}

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

MatisseParameters* Server::buildMatisseParameters(JobDefinition &job) {

    QString model = job.parametersDefinition()->model();
    QString paramName = job.parametersDefinition()->name().replace(" ", "_");
    // TODO Centraliser dans
    QString file =
            QDir::cleanPath( _xmlTool.getBasePath()
                             + QDir::separator()
                             + "users"
                             + QDir::separator()
                             + "parameters"
                             + QDir::separator()
                             + model
                             + QDir::separator()
                             + paramName + ".xml");

    qDebug() << "Chargement du fichier de paramètres : " << file;
    MatisseParameters* parameters = NULL;
    if (QFile::exists(file)) {
        parameters = new MatisseParameters(file);
    } else {
        setMessageStr(tr("Fichier de paramètres introuvable: %1").arg(file));
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

Xml& Server::xmlTool()
{
    return _xmlTool;
}

void Server::slot_currentJobProcessed()
{
    JobDefinition jobDefinition = _currentJob->jobDefinition();
    bool isCancelled=_currentJob->isCancelled();
    QString jobName = jobDefinition.name();
    jobDefinition.executionDefinition()->setExecuted(!isCancelled);
    if (!isCancelled) {
        jobDefinition.executionDefinition()->setResultFileName(_currentJob->resultFileName());
        _jobServer->sendExecutionNotification(jobName);
    }
    _currentJob->deleteLater();
    _currentJob = NULL;
    emit signal_jobProcessed(jobName, isCancelled);
}


bool Server::buildJobTask(AssemblyDefinition &assembly, JobDefinition &jobDefinition, MatisseParameters *matisseParameters)
{
    ImageProvider* imageProvider = NULL;
    QList<Processor*> processorList;
    RasterProvider* rasterProvider = NULL;

    qDebug() << "Verification de l'assemblage";



    // Verifier si les paramètres attendus sont présents pour la source
    QString sourceName = assembly.sourceDefinition()->name();
    qDebug() << "Verification présence de la source" << sourceName;
    imageProvider = _imageProviders.value(sourceName);
    if (!imageProvider) {
        setMessageStr(tr("Module source introuvable: %1").arg(sourceName));
        return false;
    }

    qDebug() << "Verification des paramètres attendus";
    QList<MatisseParameter> expectedParams = imageProvider->expectedParameters();
    foreach (MatisseParameter mp, expectedParams) {
        if (!matisseParameters->containsParam(mp.structure, mp.param)) {
            setMessageStr(tr("Paramètre requis manquant dans l'assemblage: (%1, %2) pour %3").arg(mp.structure, mp.param, sourceName));
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
        if (order=0) {
            setMessageStr(tr("Processeur défini avec un ordre incorrect: %1").arg(processorName));
            // impossible :source
            return false;
        }
        else {
            Processor *processor = _processors.value(processorName ,NULL);
            if (!processor) {
                setMessageStr(tr("Module processeur introuvable: %1").arg(processorName));
                return false;
            }

            expectedParams = processor->expectedParameters();
            foreach (MatisseParameter mp, expectedParams) {
                if (!matisseParameters->containsParam(mp.structure, mp.param)) {
                    setMessageStr(tr("Paramètre requis manquant dans l'assemblage: (%1, %2) pour %3").arg(mp.structure, mp.param, processorName));
                    return false;
                }
            }
        }
    }


    // Verifier si les paramètres attendus sont présents pour la destination
    qDebug() << "Verification présence destination";
    DestinationDefinition * destinationDef= assembly.destinationDefinition();
    if (!destinationDef) {
         setMessageStr(tr("Destination non définie"));
        return false;
    }
    quint32 order = destinationDef->order();
    QString destinationName = destinationDef->name();
    if (order <= maxOrder) {
        setMessageStr(tr("Destination définie avec un ordre incorrect: %1").arg(destinationName));
        return false;
    }
    else {
        rasterProvider = _rasterProviders.value(destinationName);
        if (!rasterProvider) {
            setMessageStr(tr("Module de destination introuvable: %1").arg(destinationName));
            return false;
        }
        expectedParams = rasterProvider->expectedParameters();
        foreach (MatisseParameter mp, expectedParams) {
            if (!matisseParameters->containsParam(mp.structure, mp.param)) {
                setMessageStr(tr("Paramètre requis manquant dans l'assemblage: (%1, %2) pour %3").arg(mp.structure, mp.param, destinationName));
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
        setMessageStr(tr("Fichier de paramètres invalide"));
        return false;
    }

    qDebug() << "Dump parametres:" << parameters->dumpStructures();

    QString assemblyName = jobDefinition.assemblyName();
    AssemblyDefinition * assemblyDefinition = xmlTool().getAssembly(assemblyName);

    if (!assemblyDefinition) {
        setMessageStr(tr("Impossible de charger l'assemblage %1").arg(assemblyName));
        return false;
    }

    if (!buildJobTask(*assemblyDefinition, jobDefinition, parameters)) {
        setMessageStr(tr("Echec d'execution de l'assemblage"));
        return false;
    }

    QThread::currentThread()->setObjectName("GUI");

    _thread = new QThread;
    _thread->setObjectName("JobTask");
    connect(_thread, SIGNAL(started()), _currentJob, SLOT(slot_start()));
    connect(_thread, SIGNAL(finished()), _currentJob, SLOT(slot_stop()));
    connect(_currentJob, SIGNAL(signal_jobStopped()), this, SLOT(slot_currentJobProcessed()));
    connect(_currentJob, SIGNAL(signal_jobIntermediateResult(QString,Image *)), this, SIGNAL(signal_jobIntermediateResult(QString,Image *)));
    _currentJob->moveToThread(_thread);
    qDebug() << "Démarrage de la tache";
    _thread->start();

    return true;

}

bool Server::isProcessingJob()
{
    return _currentJob!=NULL;
}

bool Server::cancelJob()
{
   if (_currentJob) {
       _currentJob->cancel();
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

void Server::init(){

    // Load processors
    QDir processorsDir = QDir(_xmlTool.getDllPath() + QDir::separator() +  "processors");
    setMessageStr();

    foreach (QString fileName, processorsDir.entryList(QStringList() << "*.dll", QDir::Files)) {
        qDebug() <<"Loading Processor DLL " << fileName;
        QPluginLoader loader(processorsDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
            Processor* newInstance = qobject_cast<Processor *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "Processor DLL " << newInstance->name() << " loaded.";
            _processors.insert(newInstance->name(), newInstance);

        }

    }

    // Load imageProviders
    QDir imageProvidersDir = QDir(_xmlTool.getDllPath() + QDir::separator() + "imageProviders");
    foreach (QString fileName, imageProvidersDir.entryList(QStringList() << "*.dll", QDir::Files)) {
        qDebug() <<"Loading ImageProvider DLL " << fileName;
        QPluginLoader loader(imageProvidersDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            ImageProvider* newInstance = qobject_cast<ImageProvider *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "ImageProvider DLL " << newInstance->name() << " loaded.";
            _imageProviders.insert(newInstance->name(), newInstance);

        }
    }

    // Load rasterProviders
    QDir rasterProvidersDir = QDir(_xmlTool.getDllPath() + QDir::separator() + "rasterProviders");

    foreach (QString fileName, rasterProvidersDir.entryList(QStringList() << "*.dll", QDir::Files)) {
        qDebug() <<"Loading RasterProvider DLL " << fileName;
        QPluginLoader loader(rasterProvidersDir.absoluteFilePath(fileName));
        if(QObject *pluginObject = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            RasterProvider* newInstance = qobject_cast<RasterProvider *>(pluginObject); // On réinterprète alors notre QObject
            qDebug() << "RasterProvider DLL " << newInstance->name() << " loaded.";
            _rasterProviders.insert(newInstance->name(), newInstance);

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

void JobTask::cancel()
{
    _isCancelled = true;

    qDebug() << "Demande d'arret du image provider";
    _imageProvider->askForStop();

    qDebug() << "Demande d'arret des processeurs";
    foreach (Processor *processor, _processors) {
        processor->askForStop();
    }

    qDebug() << "Demande d'arret du raster provider";
    _rasterProvider -> askForStop();


}

void JobTask::slot_start()
{
    // TODO Ask for external creation of context!
    _context = new Context;

    qDebug() << "Configuration de la source";
    _imageProvider->callConfigure(_context, _matParameters);

    qDebug() << "Configuration des Processeurs";
    foreach (Processor* processor, _processors) {
        qDebug() << "Configuration du processeur " << processor->name();
        connect(processor, SIGNAL(signal_intermediateResult(Image*)), this, SLOT(slot_intermediateResult(Image*)));
        processor->callConfigure(_context, _matParameters);
    }

    // TODO: retourner un bool au lieu de void
    qDebug() << "Configuration de la destination";
    _rasterProvider->callConfigure(_context, _matParameters);


    qDebug() << "Démarrage du raster provider";

    _rasterProvider->callStart();

    qDebug() << "Démarrage des processeurs";
    foreach (Processor *processor, _processors) {
        processor->callStart();
    }

    qDebug() << "Démarrage du image provider";
    _imageProvider->callStart();

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
    disconnect(this, SLOT(slot_intermediateResult(Image*)));

    qDebug() << "Arret du raster provider";
    _rasterProvider -> callStop();

    if (_isCancelled)
        _resultFileName = "";
    else {
        // recuperation du nom du fichier de sortie
        _resultFileName = _rasterProvider->rasterInfo().absoluteFilePath();
        qDebug()<<"Nom fichier résultat: " <<_resultFileName;
    }
    delete _context;
    emit signal_jobStopped();
}

volatile bool JobTask::isCancelled() const
{
    return _isCancelled;
}

void JobTask::slot_intermediateResult(Image *image)
{
    emit signal_jobIntermediateResult(_jobDefinition.name(), image);
}




QString JobTask::resultFileName() const
{
    return _resultFileName;
}



JobDefinition &JobTask::jobDefinition() const
{
    return _jobDefinition;
}


