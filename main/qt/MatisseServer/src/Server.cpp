#include "Server.h"
#include "AssemblyDefinition.h"

using namespace MatisseServer;

// TODO Initialiser _JobServer avec une variable des settings
Server::Server(QObject *parent) :
    QObject(parent),
    _xmlTool("../xml"),
    _jobServer(NULL)
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
        setMessageStr("Fichier de configuration introuvable: " + settingsFile.absoluteFilePath());
        return false;
    }

    if (!settingsFile.isReadable()) {
        //QMessageBox::critical(this, "Fichier de configuration illisible", "Impossible de lire le fichier:\n" + _settingsFile + "\nRelancez l'application avec un nom de fichier lisible en paramètre\nou rendez le fichier " + standardFile + " lisible!");
        setMessageStr("Fichier de configuration illisible: " + settingsFile.absoluteFilePath());
        return false;
    }

    _xmlTool.readMatisseGuiSettings(settings);
    if (( _xmlTool.getBasePath()) == "") {
        //QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de XmlRootDir ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
        setMessageStr("XmlRootDir introuvable dans le fichier de configuration: " + settingsFile.absoluteFilePath());
        return false;
    }

    if ((_xmlTool.getDataPath()) == "") {
        //QMessageBox::critical(this, "Fichier de configuration incorrect", "La valeur de XmlRootDir ne peut être déterminée.\nRelancez l'application avec un paramètre XmlRootDir valide\ndans le fichier de configuration!");
        setMessageStr("DataRootDir introuvable dans le fichier de configuration: " + settingsFile.absoluteFilePath());
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

MatisseParameters* Server::buildMosaicParameters(JobDefinition &job) {

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
        setMessageStr("Fichier de paramètres introuvable: " + file);
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


bool Server::checkAssembly(MatisseParameters *mosaicParameters, AssemblyDefinition &assembly)
{
    qDebug() << "Verification de l'assemblage";
    setMessageStr();
    if (!mosaicParameters) {
        setMessageStr("Fichier de paramètres invalide");
        return false;
    }


    // Verifier si les paramètres attendus sont présents pour la source
    QString sourceName = assembly.sourceDefinition()->name();
    qDebug() << "Verification présence de la source" << sourceName;
    ImageProvider* imageProvider = _imageProviders.value(sourceName);
    if (!imageProvider) {
        setMessageStr("Module source introuvable: " + sourceName);
        return false;
    }

    qDebug() << "Verification des paramètres attendus";
    QList<MatisseParameter> expectedParams = imageProvider->expectedParameters();
    foreach (MatisseParameter mp, expectedParams) {
        if (!mosaicParameters->containsParam(mp.structure, mp.param)) {
            setMessageStr("Paramètre requis manquant dans l'assemblage: (" + mp.structure + ", " + mp.param + ") pour " + sourceName);
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
            setMessageStr("Processeur défini avec un ordre incorrect: " + processorName);
            // impossible :source
            return false;
        }
        else {
            Processor *processor = _processors.value(processorName ,NULL);
            if (!processor) {
                setMessageStr("Module processeur introuvable: " + processorName);
                return false;
            }

            expectedParams = processor->expectedParameters();
            foreach (MatisseParameter mp, expectedParams) {
                if (!mosaicParameters->containsParam(mp.structure, mp.param)) {
                    setMessageStr("Paramètre requis manquant dans l'assemblage: (" + mp.structure + ", " + mp.param + ") pour " + processorName);
                    return false;
                }
            }
        }
    }

    // Verifier si les paramètres attendus sont présents pour la destination
    qDebug() << "Verification présence destination";
    DestinationDefinition * destinationDef= assembly.destinationDefinition();
    if (!destinationDef) {
         setMessageStr("Destination non définie");
        return false;
    }
    quint32 order = destinationDef->order();
    QString destinationName = destinationDef->name();
    if (order <= maxOrder) {
        setMessageStr("Destination définie avec un ordre incorrect: " + destinationName);
        return false;
    }
    else {
        RasterProvider * rasterProvider = _rasterProviders.value(destinationName);
        if (!rasterProvider) {
            setMessageStr("Module de destination introuvable: " + destinationName);
            return false;
        }
        expectedParams = rasterProvider->expectedParameters();
        foreach (MatisseParameter mp, expectedParams) {
            if (!mosaicParameters->containsParam(mp.structure, mp.param)) {
                setMessageStr("Paramètre requis manquant dans l'assemblage: (" + mp.structure + ", " + mp.param + ") pour " + destinationName);
                return false;
            }
        }
    }

    return true;

}

bool Server::processJob(JobDefinition &job)
{
    setMessageStr();
    MatisseParameters* parameters = buildMosaicParameters(job);
    QString assemblyName = job.assemblyName();
    AssemblyDefinition * assembly = xmlTool().getAssembly(assemblyName);
    // normalement il existe!!!!
    if (!assembly) {
        setMessageStr("Impossible de charger l'assemblage " + assemblyName);
        return false;
    }

    if (!checkAssembly(parameters, *assembly)) {
        setMessageStr("Echec de verification de l'assemblage...");
        return false;
    }

    // Recuperation de l'ordre du raster...
    quint32 destinationOrder = assembly->destinationDefinition()->order();

    qDebug() << "Dump parametres:" << parameters->dumpStructures();

    // Ports par ordre
    QHash<quint32, QList<ImageSetPort *>* > inProcessorPortsByOrder;
    QHash<quint32, QList<ImageSetPort *>* > outProcessorPortsByOrder;

    qDebug() << "Creation des ImageSet ";
    QList<ConnectionDefinition*> connectionDefs = assembly->connectionDefs();
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
            QString sourceName = assembly->sourceDefinition()->name();
            qDebug() << "  Creation de out port imageProvider " << sourceName;
            ImageProvider *provider = _imageProviders.value(sourceName);
            if (provider) {
                outPort->imageSet = provider->imageSet(outPortNumber);
            }
        } else {
            qDebug() << "  Creation de out port processor ";
            // Creation d'un imageset vide
            outPort->imageSet = new ImageSet();

        }
        if (endOrder == destinationOrder) {
            QString destinationName = assembly->destinationDefinition()->name();
            qDebug() << "  Creation de in port rasterProvider " << destinationName;
            RasterProvider *provider = _rasterProviders.value(destinationName);
            if (provider) {
                provider->setImageSet(inPort->imageSet);
            }

        }

        inPort->imageSet=outPort->imageSet;
        outProcessorPorts->append(outPort);
        inProcessorPorts->append(inPort);
    }

    // TODO Ask for external creation of context!
    Context* context = new Context();


    qDebug() << "Configuration de la source";
    ImageProvider *imageProvider =  _imageProviders.value(assembly->sourceDefinition()->name());

    if (imageProvider) {
        if (!imageProvider->callConfigure(context, parameters)) {
            setMessageStr("Erreur de configuration de la source. Arrêt");
            return false;
        }
    } else {
        setMessageStr("Source introuvable...");
        return false;
    }

    qDebug() << "Configuration de la destination";
    RasterProvider *rasterProvider =  _rasterProviders.value(assembly->destinationDefinition()->name());

    if (rasterProvider) {
        rasterProvider->callConfigure(context, parameters);
    } else {
        setMessageStr("Destination introuvable...");
        return false;
    }

    qDebug() << "Configuration des Processeurs";
    QList<ProcessorDefinition*> processorDefs= assembly->processorDefs();
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
            if (!processor) {
                setMessageStr("Processeur " + processorName + " introuvable!");
                return false;
            }
            else {
                qDebug() << "Configuration du processeur " << processor->name();

                processor->callConfigure(context, parameters);
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
            }
        }
    }

    qDebug() << "Démarrage du raster provider";
    //rasterProvider =  _rasterProviders.value(assembly->destinationDefinition()->name());
    rasterProvider->callStart();

    qDebug() << "Démarrage des processeurs";
    foreach (ProcessorDefinition* procDef, processorDefs) {
        quint32 order = procDef->order();
        QString processorName = procDef->name();
        if (order==0) {
            // impossible :source
        }
        else {
            Processor *processor = _processors.value(processorName);
            processor->callStart();
        }
    }

    qDebug() << "Démarrage du image provider";
    //imageProvider =  _imageProviders.value(assembly->sourceDefinition()->name());
    imageProvider->callStart();

    qDebug() << "Arret du image provider";
    imageProvider->callStop();

    qDebug() << "Arret des processeurs";
    foreach (ProcessorDefinition* procDef, processorDefs) {
        quint32 order = procDef->order();
        QString processorName = procDef->name();
        if (order==0) {
            // impossible :source
        }
        else {
            Processor *processor = _processors.value(processorName);
            processor->callStop();
        }
    }

    qDebug() << "Arret du raster provider";
    rasterProvider -> callStop();
    // recuperation du nom du fichier de sortie
    job.executionDefinition()->setResultFileName(rasterProvider->rasterInfo().absoluteFilePath());
    emit signal_jobProcessed(job.name());
    _jobServer->sendExecutionNotification(job.name());

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
    QDir processorsDir = QDir("../dll/processors");
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
    QDir imageProvidersDir = QDir("../dll/imageProviders");

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
    QDir rasterProvidersDir = QDir("../dll/rasterProviders");

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








