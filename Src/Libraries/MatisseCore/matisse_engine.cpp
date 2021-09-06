#include "matisse_engine.h"

namespace matisse {

MatisseEngine::MatisseEngine(QObject *_parent, bool _is_server_mode) :
    QObject(_parent),
    m_is_server_mode(_is_server_mode),
    m_job_launcher(NULL),
    m_job_server(NULL),
    m_current_job(NULL),
    m_thread(NULL)
{

}


MatisseEngine::~MatisseEngine()
{
    if (m_job_server) {
        delete m_job_server;
    }
    if(m_thread)
        m_thread->exit();
    if(m_current_job)
        m_current_job->stop(true);
}


QList<Processor*> const MatisseEngine::getAvailableProcessors() {
    return m_processors.values();
}

QList<InputDataProvider*> const MatisseEngine::getAvailableInputDataProviders() {
    return m_input_data_providers.values();
}

const QList<OutputDataWriter *> MatisseEngine::getAvailableOutputDataWriters()
{
    return m_output_data_writers.values();
}

void MatisseEngine::addParametersForInputDataProvider(QString _name)
{
    qDebug() << "Loading parameters for image provider " << _name;

    if (!m_input_data_providers.contains(_name)) {
        qCritical() << QString("Image provider '%1' could not be found.").arg(_name);
        return;
    }

    InputDataProvider* source = m_input_data_providers.value(_name);
    QList<MatisseParameter> expected_params = source->expectedParameters();
    m_expected_parameters_by_module.insert(_name, expected_params);

    foreach(MatisseParameter expected_param, expected_params) {
        qDebug() << "Source expects param " << expected_param.m_structure << expected_param.m_param;
        m_dico_param_mgr->addUserModuleForParameter(_name, expected_param.m_structure, expected_param.m_param);
    }

}

void MatisseEngine::addParametersForProcessor(QString _name)
{
    qDebug() << "Loading parameters for Processor " << _name;

    if (!m_processors.contains(_name)) {
        qCritical() << QString("Processor '%1' could not be found.").arg(_name);
        return;
    }

    Processor* processor = m_processors.value(_name);
    QList<MatisseParameter> expected_params = processor->expectedParameters();
    m_expected_parameters_by_module.insert(_name, expected_params);

    foreach(MatisseParameter expected_param, expected_params) {
        qDebug() << "Processor expects param " << expected_param.m_structure << expected_param.m_param;
        m_dico_param_mgr->addUserModuleForParameter(_name, expected_param.m_structure, expected_param.m_param);
    }

}


void MatisseEngine::addParametersForOutputDataWriter(QString _name)
{
    qDebug() << "Loading parameters for raster provider " << _name;

    if (!m_output_data_writers.contains(_name)) {
        qCritical() << QString("Raster provider '%1' could not be found.").arg(_name);
        return;
    }

    OutputDataWriter* destination = m_output_data_writers.value(_name);
    QList<MatisseParameter> expected_params = destination->expectedParameters();
    m_expected_parameters_by_module.insert(_name, expected_params);

    foreach(MatisseParameter expected_param, expected_params) {
        qDebug() << "Destination expects param " << expected_param.m_structure << expected_param.m_param;
        m_dico_param_mgr->addUserModuleForParameter(_name, expected_param.m_structure, expected_param.m_param);
    }

}

bool MatisseEngine::removeModuleAndExpectedParameters(QString _name)
{
    qDebug() << QString("Remove module '%1' from expected parameters list").arg(_name);

    if (!m_expected_parameters_by_module.contains(_name)) {
        qCritical() << "Unknown module" << _name;
        return false;
    }

    QList<MatisseParameter> expected_parameters = m_expected_parameters_by_module.value(_name);
    foreach(MatisseParameter param, expected_parameters) {
        m_dico_param_mgr->removeUserModuleForParameter(_name, param.m_structure, param.m_param);
    }

    return true;
}

MatisseParameters* MatisseEngine::buildMatisseParameters(JobDefinition *_job) {


    QString file = ProcessDataManager::instance()->getJobParametersFilePath(_job->name());

    qDebug() << "Chargement du fichier de paramètres : " << file;
    MatisseParameters* parameters = NULL;
    if (QFile::exists(file)) {
        parameters = new MatisseParameters(file);
    } else {
        setMessageStr(tr("Cannot find parameters file: %1").arg(file));
    }

    return parameters;

}

void MatisseEngine::setMessageStr(QString _message_Str, bool _error)
{
    m_message_str = _message_Str;
    m_error_flag = _error;

    if (!m_message_str.isEmpty()) {
        if (m_error_flag) {
            qDebug() << "[ERREUR SERVEUR]:" << m_message_str;
        } else {
            qDebug() << "[MESSAGE SERVEUR]:" << m_message_str;
        }
    } else {
        // si on reinitialise, pas d'erreur...
        m_error_flag = false;
    }
}

void MatisseEngine::sl_currentJobProcessed()
{
    qDebug() << "Current job processed, updating job data...";

    if(m_current_job == NULL) {
        // inconsistent case
        qCritical() << "Job processed but pointer to current job is null";
        return;
    }

    JobDefinition *job_definition = m_current_job->jobDefinition();
    bool is_cancelled = m_current_job->isCancelled();
    QString job_name = job_definition->name();
    job_definition->executionDefinition()->setExecuted(!is_cancelled);
    qDebug() << "Task job definition execution status : " << job_definition->executionDefinition()->executed();
    if (!is_cancelled) {
        if (m_current_job->resultFileNames().isEmpty()) {
            qWarning() << "No result file";
        }

        for (QString file_name:m_current_job->resultFileNames()) {
            qDebug() << QString("Result file : %1").arg(file_name);
        }

        job_definition->executionDefinition()->setResultFileNames(m_current_job->resultFileNames());
        m_job_server->sendExecutionNotification(job_name);
    } else {
        qDebug() << "Job canceled, no results";
    }

    // debug
    job_definition->setComment("Test comment");

    disconnect(this, SLOT(sl_currentJobProcessed()));
    disconnect(this, SIGNAL(si_jobShowImageOnMainView(QString,Image *)));
    disconnect(this, SIGNAL(si_userInformation(QString)));
    disconnect(this, SIGNAL(si_processCompletion(quint8)));

    m_current_job->deleteLater();
    m_current_job = NULL;
    emit si_jobProcessed(job_name, is_cancelled);
}

bool MatisseEngine::buildJobTask(AssemblyDefinition *_assembly, JobDefinition *_job_definition, MatisseParameters *_matisse_parameters)
{
    InputDataProvider* input_data_provider = NULL;
    QList<Processor*> processor_list;
    OutputDataWriter* output_data_writer = NULL;

    qDebug() << "Check assembly";


    // Verifier si les paramètres attendus sont présents pour la source
    QString source_name = _assembly->sourceDefinition()->name();
    qDebug() << "Verification présence de la source" << source_name;
    input_data_provider = m_input_data_providers.value(source_name);
    if (!input_data_provider) {
        setMessageStr(tr("Source module not found: %1").arg(source_name));
        return false;
    }

    qDebug() << "Verification des paramètres attendus";
    QList<MatisseParameter> expected_params = input_data_provider->expectedParameters();
    foreach (MatisseParameter mp, expected_params) {
        if (!_matisse_parameters->containsParam(mp.m_structure, mp.m_param)) {
            setMessageStr(tr("Required parameter not found: (%1, %2) for %3").arg(mp.m_structure, mp.m_param, source_name));
            return false;
        }
    }

    quint32 max_order = 0;
    // Verifier si les paramètres attendus sont présents pour les algorithmes
    qDebug() << "Verification présence des processeurs";
    QList<ProcessorDefinition*> processor_defs = _assembly->processorDefs();
    foreach (ProcessorDefinition* proc_def, processor_defs) {
        quint32 order = proc_def->order();
        max_order = qMax(max_order, order);
        QString processor_name = proc_def->name();
        qDebug() << "Processeur" << processor_name;
        if (order==0) {
            setMessageStr(tr("Processor defined in wrong order: %1").arg(processor_name));
            // impossible :source
            return false;
        }
        else {
            Processor *processor = m_processors.value(processor_name ,NULL);
            if (!processor) {
                setMessageStr(tr("Cannot find processor module: %1").arg(processor_name));
                return false;
            }

            expected_params = processor->expectedParameters();
            foreach (MatisseParameter mp, expected_params) {
                if (!_matisse_parameters->containsParam(mp.m_structure, mp.m_param)) {
                    setMessageStr(tr("Required parameter not found in assembly: (%1, %2) for %3").arg(mp.m_structure, mp.m_param, processor_name));
                    return false;
                }
            }
        }
    }


    // Verifier si les paramètres attendus sont présents pour la destination
    qDebug() << "Verification présence destination";
    DestinationDefinition * destination_def= _assembly->destinationDefinition();
    if (!destination_def) {
         setMessageStr(tr("Destination not defined"));
        return false;
    }
    quint32 order = destination_def->order();
    QString destination_name = destination_def->name();
    if (order <= max_order) {
        setMessageStr(tr("Destination defined with wrong order: %1").arg(destination_name));
        return false;
    }
    else {
        output_data_writer = m_output_data_writers.value(destination_name);
        if (!output_data_writer) {
            setMessageStr(tr("Cannot find destionation module: %1").arg(destination_name));
            return false;
        }
        expected_params = output_data_writer->expectedParameters();
        foreach (MatisseParameter mp, expected_params) {
            if (!_matisse_parameters->containsParam(mp.m_structure, mp.m_param)) {
                setMessageStr(tr("Cannot find required parameter for assembly: (%1, %2) for %3").arg(mp.m_structure, mp.m_param, destination_name));
                return false;
            }
        }
    }


    qDebug() << "Creation des ImageSet ";
    // Recuperation de l'ordre du raster...
    quint32 destination_order = _assembly->destinationDefinition()->order();

    // Ports par ordre
    QHash<quint32, QList<ImageSetPort *>* > in_processor_ports_by_order;
    QHash<quint32, QList<ImageSetPort *>* > out_processor_ports_by_order;


    QList<ConnectionDefinition*> connection_defs = _assembly->connectionDefs();
    foreach (ConnectionDefinition* con_def, connection_defs) {
        quint32 startOrder = con_def->startOrder();
        quint32 endOrder = con_def->endOrder();
        QList<ImageSetPort *> *out_processor_ports = out_processor_ports_by_order.value(startOrder, NULL);
        if (!out_processor_ports) {
            out_processor_ports = new QList<ImageSetPort *>;
            out_processor_ports_by_order.insert(startOrder, out_processor_ports);
        }
        QList<ImageSetPort *> *in_processor_ports = in_processor_ports_by_order.value(endOrder, NULL);
        if (!in_processor_ports) {
            in_processor_ports = new QList<ImageSetPort *>;
            in_processor_ports_by_order.insert(endOrder, in_processor_ports);
        }
        // out source
        ImageSetPort *out_port = new ImageSetPort();
        ImageSetPort *in_port = new ImageSetPort();
        quint32 out_port_number = con_def->startLine();
        quint32 in_port_number = con_def->endLine();
        out_port->port_number = out_port_number;
        in_port->port_number = in_port_number;
        if (startOrder==0) {
            out_port->image_set = input_data_provider->imageSet(out_port_number);
        } else {
            qDebug() << "  Creation de out port processor ";
            // Creation d'un imageset vide
            out_port->image_set = new ImageSet();

        }

        in_port->image_set=out_port->image_set;


        out_processor_ports->append(out_port);
        in_processor_ports->append(in_port);
    }



    qDebug() << "Configuration des Processeurs";
    foreach (ProcessorDefinition* proc_def, processor_defs) {
        quint32 order = proc_def->order();
        QString processor_name = proc_def->name();
        if (order==0) {
            // impossible :source
        } else if (order == destination_order) {
            // impossible : destination
        }
        else {
            Processor *processor = m_processors.value(processor_name);

            qDebug() << "Fin assemblage du processeur " << processor_name;
            QList<ImageSetPort *> * in_ports =  in_processor_ports_by_order.value(order);
            if (in_ports) {
                qDebug() << "  Ajout des connections entrantes de " << processor_name << " - nb: " << in_ports->size() ;
                processor->setInputPortList(in_ports);
                foreach (ImageSetPort* in_port, *in_ports)
                {
                    qDebug() << "  Port entrant " << in_port->port_number;
                    in_port->image_set->setOutPort(in_port);
                    in_port->image_listener = processor;
                }
            }
            QList<ImageSetPort *> * out_ports =  out_processor_ports_by_order.value(order);
            if (out_ports) {
                qDebug() << "  Ajout des connections sortantes de " << processor_name << " - nb: " << out_ports->size() ;
                processor->setOutputPortList(out_ports);
            }
            processor_list.append(processor);
        }
    }

    qDebug() << "Configuration du RasterProvider";

    order = destination_def->order();
    QList<ImageSetPort *> * in_ports =  in_processor_ports_by_order.value(order);
    if (in_ports && in_ports->size()==1) {
        ImageSetPort* in_port = in_ports->at(0);
        qDebug() << "Raster  Port entrant " << in_port->port_number;
        output_data_writer->setImageSet(in_port->image_set);
        in_port->image_set->setOutPort(in_port);
        in_port->image_listener = output_data_writer;
    }


    m_current_job = new JobTask(input_data_provider, processor_list, output_data_writer, _job_definition, _matisse_parameters);
    m_current_job->setIsServerMode(m_is_server_mode);
    m_current_job->setJobLauncher(m_job_launcher);

    return true;

}



bool MatisseEngine::processJob(JobDefinition *_job_definition)
{
    setMessageStr();

    if (m_current_job) {
        qDebug() << "A Thread is running";
        // TODO Queue jobs?
        return false;
    }

     setMessageStr();

    // Deleted by JobTask
    MatisseParameters* parameters = buildMatisseParameters(_job_definition);



    if (!parameters) {
        setMessageStr(tr("Invalid parameters file"));
        return false;
    }

    qDebug() << "Dump parametres:" << parameters->dumpStructures();

    QString assembly_name = _job_definition->assemblyName();
    AssemblyDefinition * assembly_definition = ProcessDataManager::instance()->getAssembly(assembly_name);

    if (!assembly_definition) {
        setMessageStr(tr("Cannot load assembly %1").arg(assembly_name));
        return false;
    }

    if (!buildJobTask(assembly_definition, _job_definition, parameters)) {
        setMessageStr(tr("Running assembly failed"));
        return false;
    }

    QThread::currentThread()->setObjectName("GUI");

    // TODO : penser a deleter le pointeur
    m_thread = new QThread;
    m_thread->setObjectName("JobTask");
    connect(m_thread, SIGNAL(started()), m_current_job, SLOT(sl_start()));
    connect(m_thread, SIGNAL(finished()), m_current_job, SLOT(sl_stop()));
    connect(m_current_job, SIGNAL(si_jobStopped()), this, SLOT(sl_currentJobProcessed()));
    connect(m_current_job, SIGNAL(si_jobShowImageOnMainView(QString,Image *)), this, SIGNAL(si_jobShowImageOnMainView(QString,Image *)));
    connect(m_current_job, SIGNAL(si_userInformation(QString)), this, SIGNAL(si_userInformation(QString)));
    connect(m_current_job, SIGNAL(si_processCompletion(quint8)), this, SIGNAL(si_processCompletion(quint8)));
    m_current_job->moveToThread(m_thread);
    qDebug() << "Démarrage de la tache";
    m_thread->start();

    return true;

}

bool MatisseEngine::isProcessingJob()
{
    return m_current_job!=NULL;
}

bool MatisseEngine::stopJob(bool _cancel)
{
    if (m_current_job) {
        m_current_job->stop(_cancel);

        disconnect(this, SLOT(sl_currentJobProcessed()));
        disconnect(this, SIGNAL(si_jobShowImageOnMainView(QString,Image *)));
        disconnect(this, SIGNAL(si_userInformation(QString)));
        disconnect(this, SIGNAL(si_processCompletion(quint8)));

        qDebug() << "Fin du Thread" ;
        m_thread->quit();
    }
    return true;
}

bool MatisseEngine::errorFlag()
{
    return m_error_flag;
}

QString MatisseEngine::messageStr()
{
    return m_message_str;
}

#ifdef WIN32
#define SHARED_DLL_EXT "*.dll"
#else
#define SHARED_DLL_EXT "*.so"
#endif

void MatisseEngine::init(){

    SystemDataManager* system_data_manager = SystemDataManager::instance();

    m_job_server = new JobServer(system_data_manager->port());

    loadParametersDictionnary();

    // Load processors
    QDir processors_dir = QDir(system_data_manager->getDllPath() + QDir::separator() +  "processors");
    setMessageStr();



    foreach (QString file_name, processors_dir.entryList(QStringList() << SHARED_DLL_EXT , QDir::Files)) {
        qDebug() <<"Loading Processor DLL " << file_name;
        QPluginLoader loader(processors_dir.absoluteFilePath(file_name));
        if(QObject *plugin_object = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.
            Processor* new_instance = qobject_cast<Processor *>(plugin_object); // On réinterprète alors notre QObject
            qDebug() << "Processor DLL " << new_instance->name() << " loaded.";
            m_processors.insert(new_instance->name(), new_instance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << file_name;
            if(loader.isLoaded())
            {
                QObject *plugin_object = loader.instance();
                if(plugin_object)
                {
                    Processor* new_instance = qobject_cast<Processor *>(plugin_object); // On réinterprète alors notre QObject
                    qDebug() << "Processor DLL " << new_instance->name() << " loaded.";
                    m_processors.insert(new_instance->name(), new_instance);

                }
                else
                {
                    QString problem = loader.errorString();
                    qDebug() <<  "Plugin load problem retry: " << problem;
                }
            }
        }

    }

    // Load input data providers
    QDir input_data_providers_dir = QDir(system_data_manager->getDllPath() + QDir::separator() + "imageProviders");
    foreach (QString file_name, input_data_providers_dir.entryList(QStringList() << SHARED_DLL_EXT, QDir::Files)) {
        qDebug() <<"Loading InputDataProvider DLL " << file_name;
        QPluginLoader loader(input_data_providers_dir.absoluteFilePath(file_name));
        if(QObject *plugin_object = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            InputDataProvider* new_instance = qobject_cast<InputDataProvider *>(plugin_object); // On réinterprète alors notre QObject
            qDebug() << "InputDataProvider DLL " << new_instance->name() << " loaded.";
            m_input_data_providers.insert(new_instance->name(), new_instance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << file_name;
        }

    }

    // Load output data writers
    QDir output_data_writers_dir = QDir(system_data_manager->getDllPath() + QDir::separator() + "rasterProviders");

    foreach (QString file_name, output_data_writers_dir.entryList(QStringList() << SHARED_DLL_EXT, QDir::Files)) {
        qDebug() <<"Loading OutputDataWriter DLL " << file_name;
        QPluginLoader loader(output_data_writers_dir.absoluteFilePath(file_name));
        if(QObject *plugin_object = loader.instance()) { // On prend l'instance de notre plugin sous forme de QObject. On vérifie en même temps s'il n'y a pas d'erreur.

            OutputDataWriter* new_instance = qobject_cast<OutputDataWriter *>(plugin_object); // On réinterprète alors notre QObject
            qDebug() << "OutputDataWriter DLL " << new_instance->name() << " loaded.";
            m_output_data_writers.insert(new_instance->name(), new_instance);

        } else {
            QString problem = loader.errorString();
            qDebug() <<  "Plugin load problem: " << problem;
            qCritical() << "Could not load DLL " << file_name;
        }

    }

}


JobTask::JobTask(InputDataProvider* _input_data_provider, QList<Processor*> _processors, OutputDataWriter* _output_data_writer,
                 JobDefinition *_job_definition, MatisseParameters *_parameters )
    : m_user_log_file(NULL),
      m_input_data_provider(_input_data_provider),
      m_processors(_processors),
      m_output_data_writer(_output_data_writer),
      m_job_definition(_job_definition),
      m_mat_parameters(_parameters),
      m_is_cancelled(false)
{
    // Create the log file in the output folder
    QString dataset_dir = m_mat_parameters->getStringParamValue("dataset_param", "dataset_dir");

    QDir output_dir(m_mat_parameters->getStringParamValue("dataset_param", "output_dir"));
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
    delete m_mat_parameters;
}

void JobTask::stop(bool _cancel)
{
    m_is_cancelled = _cancel;

    m_input_data_provider->askToStop(_cancel);

    foreach (Processor *processor, m_processors) {
        processor->askToStop(_cancel);
    }

    m_output_data_writer->askToStop(_cancel);
}

void JobTask::sl_start()
{

    // TODO Ask for external creation of context!
    bool ok;
    m_context = new Context;

    connect(m_input_data_provider, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
    connect(m_input_data_provider, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
    if (!m_is_server_mode) {
      connect(m_input_data_provider, SIGNAL(si_showInformationMessage(QString, QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString, QString)));
      connect(m_input_data_provider, SIGNAL(si_showErrorMessage(QString, QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString, QString)));
      connect(m_input_data_provider, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
      connect(m_input_data_provider, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
      connect(m_input_data_provider, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
      connect(m_input_data_provider, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
    }

    ok = m_input_data_provider->callConfigure(m_context, m_mat_parameters);
    if (!ok) {
      /* Bad parameters or dataset not found */
      qCritical() << "Error on raster provider configuration";
      if (m_is_server_mode) { // for non-regression with client mode (is there a reason why the task should continue in that case ?) 
          sl_fatalError();
      }
      return;
    }


    qDebug() << "Configuration des Processeurs";
    foreach (Processor* processor, m_processors) {
        qDebug() << "Configuration du processeur " << processor->name();
        connect(processor, SIGNAL(si_showImageOnMainView(Image*)), this, SLOT(sl_showImageOnMainView(Image*)));
        connect(processor, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
        connect(processor, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
        connect(processor, SIGNAL(si_fatalError()), this, SLOT(sl_fatalError()));
        connect(processor, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
        if (!m_is_server_mode) {
          connect(processor, SIGNAL(si_showInformationMessage(QString,QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString,QString)));
          connect(processor, SIGNAL(si_showErrorMessage(QString,QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString,QString)));
          connect(processor, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
          connect(processor, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
          connect(processor, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
        }
        processor->callConfigure(m_context, m_mat_parameters);
    }

    qDebug() << "Configuration de la destination";
    connect(m_output_data_writer, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
    connect(m_output_data_writer, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
    connect(m_output_data_writer, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
    if (!m_is_server_mode) {
      connect(m_output_data_writer, SIGNAL(si_showInformationMessage(QString,QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString,QString)));
      connect(m_output_data_writer, SIGNAL(si_showErrorMessage(QString,QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString,QString)));
      connect(m_output_data_writer, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
      connect(m_output_data_writer, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
      connect(m_output_data_writer, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
    }
    ok = m_output_data_writer->callConfigure(m_context, m_mat_parameters);
    if (!ok) {
        qDebug() << "Error on raster provider configuration";
        return;
    }


    qDebug() << "Démarrage du raster provider";
    ok = m_output_data_writer->callStart();
    if (!ok) {
        qDebug() << "Error on raster provider start";
        return;
    }

    qDebug() << "Démarrage des processeurs";
    foreach (Processor *processor, m_processors) {
        ok = processor->callStart();
        if (!ok) {
            qDebug() << "Error on processor start: " << processor->name();
            return;
        }
    }

    qDebug() << "Démarrage du image provider";
    ok = m_input_data_provider->callStart();
    if (!ok) {
        qDebug() << "Error on image provider start";
        return;
    }

    qDebug() << "Fin de sl_start";

    QThread::currentThread()->exit();
}

void JobTask::sl_stop()
{

    qDebug() << "Arret du input data provider";
    m_input_data_provider->callStop();

    qDebug() << "Arret des processeurs";
    foreach (Processor *processor, m_processors) {
        processor->callStop();
    }
    disconnect(this, SLOT(sl_showImageOnMainView(Image*)));

    qDebug() << "Arret du output data writer";
    m_output_data_writer->callStop();

    /* Disconnecting user information signals */
    disconnect(this, SLOT(sl_userInformation(QString)));
    disconnect(this, SLOT(sl_processCompletion(quint8)));

    m_result_file_names.clear();
    if (!m_is_cancelled) {
        // recuperation du nom du fichier de sortie

        foreach (QFileInfo raster_info, m_output_data_writer->rastersInfo()) {
            m_result_file_names << raster_info.absoluteFilePath();
        }
    }

    // Disconnect everything
    disconnect(m_input_data_provider, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
    disconnect(m_input_data_provider, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
    disconnect(m_input_data_provider, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
    if (!m_is_server_mode) {
      disconnect(m_input_data_provider, SIGNAL(si_showInformationMessage(QString,QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString,QString)));
      disconnect(m_input_data_provider, SIGNAL(si_showErrorMessage(QString,QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString,QString)));
      disconnect(m_input_data_provider, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
      disconnect(m_input_data_provider, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
      disconnect(m_input_data_provider, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
    }

    foreach (Processor* processor, m_processors) {
        if(!processor->okStatus())
            m_is_cancelled = true;
        disconnect(processor, SIGNAL(si_showImageOnMainView(Image*)), this, SLOT(sl_showImageOnMainView(Image*)));
        disconnect(processor, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
        disconnect(processor, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
        disconnect(processor, SIGNAL(si_fatalError()), this, SLOT(sl_fatalError()));
        disconnect(processor, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
        if (!m_is_server_mode) {
          disconnect(processor, SIGNAL(si_showInformationMessage(QString,QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString,QString)));
          disconnect(processor, SIGNAL(si_showErrorMessage(QString,QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString,QString)));
          disconnect(processor, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
          disconnect(processor, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
          disconnect(processor, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
        }
    }

    disconnect(m_output_data_writer, SIGNAL(si_userInformation(QString)), this, SLOT(sl_userInformation(QString)));
    disconnect(m_output_data_writer, SIGNAL(si_processCompletion(quint8)), this, SLOT(sl_processCompletion(quint8)));
    disconnect(m_output_data_writer, SIGNAL(si_addToLog(QString)), this, SLOT(sl_logToFile(QString)));
    if (!m_is_server_mode) {
      disconnect(m_output_data_writer, SIGNAL(si_showInformationMessage(QString,QString)), m_job_launcher, SLOT(sl_showInformationMessage(QString,QString)));
      disconnect(m_output_data_writer, SIGNAL(si_showErrorMessage(QString,QString)), m_job_launcher, SLOT(sl_showErrorMessage(QString,QString)));
      disconnect(m_output_data_writer, SIGNAL(si_show3DFileOnMainView(QString)), m_job_launcher, SLOT(sl_show3DFileOnMainView(QString)));
      disconnect(m_output_data_writer, SIGNAL(si_addRasterFileToMap(QString)), m_job_launcher, SLOT(sl_addRasterFileToMap(QString)));
      disconnect(m_output_data_writer, SIGNAL(si_addToLog(QString)), m_job_launcher, SLOT(sl_addToLog(QString)));
    }

    if(m_context != NULL)
    {
        delete m_context;
        m_context = NULL;
        emit si_jobStopped();
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
    return m_is_cancelled;
}

void JobTask::sl_showImageOnMainView(Image *_image)
{
    emit si_jobShowImageOnMainView(m_job_definition->name(), _image);
}

void JobTask::sl_userInformation(QString _user_text)
{
    emit si_userInformation(_user_text);
}

void JobTask::sl_processCompletion(quint8 _percent_complete)
{
    emit si_processCompletion(_percent_complete);
}

void JobTask::sl_fatalError()
{
    m_is_cancelled = true;
    stop(true);
    m_is_cancelled = true;
    emit si_jobStopped();
    sl_stop();
}

void JobTask::sl_logToFile(QString _log_info)
{
    QTextStream out_stream(m_user_log_file);
    out_stream << _log_info << "\n";
}

void JobTask::setJobLauncher(QObject *_job_launcher)
{
    m_job_launcher = _job_launcher;
}

void JobTask::setIsServerMode(bool _is_server_mode) 
{
  m_is_server_mode = _is_server_mode;
}

QStringList JobTask::resultFileNames() const
{
    return m_result_file_names;
}

JobDefinition *JobTask::jobDefinition() const
{
    return m_job_definition;
}


bool MatisseEngine::loadParametersDictionnary()
{
    QXmlSchema dictionnary_schema;

    SystemDataManager* system_data_manager = SystemDataManager::instance();

    qDebug() << "Loading MatisseParametersDictionnary.xsd schema...";

    QString dico_schema_path = system_data_manager->getBinRootDir() +
                    QDir::separator() + "schemas" + QDir::separator() +
                    "MatisseParametersDictionnary.xsd";
    QFile dico_xsd_file(dico_schema_path);

    if (!dico_xsd_file.exists()) {
        qFatal("%s\n",QString("Error finding ").append(dico_xsd_file.fileName()).toStdString().c_str());
    }

    if (!dico_xsd_file.open(QIODevice::ReadOnly)) {
        qFatal("%s\n","Error opening MatisseParametersDictionnary.xsd");
    }

    if (!dictionnary_schema.load(&dico_xsd_file, QUrl::fromLocalFile(dico_xsd_file.fileName()))) {
        qFatal("%s\n","Error loading ParametersDictionnary.xsd");
    }

    if (!dictionnary_schema.isValid()) {
        qFatal("%s\n","Error ParametersDictionnary.xsd is not valid");
    }

    qDebug() << "MatisseParametersDictionnary.xsd is a valid schema";
    dico_xsd_file.close();

    qDebug() << "Loading dictionnary file...";

    QString dico_path = system_data_manager->getBinRootDir() +
                        QDir::separator() + "config" + QDir::separator() +
                        "MatisseParametersDictionnary.xml";
    QFile dico_xml_file(dico_path);

    if (!dico_xml_file.exists()) {
        qFatal("%s\n",QString("Error finding").append(dico_xml_file.fileName()).toStdString().c_str());
    }

    if (!dico_xml_file.open(QIODevice::ReadOnly)) {
        qFatal("%s\n","Error opening MatisseParametersDictionnary.xml");
    }

    QXmlSchemaValidator validator(dictionnary_schema);
    if (!validator.validate(&dico_xml_file, QUrl::fromLocalFile(dico_xml_file.fileName()))) {
        qFatal("%s\n","Dictionnary XML file does not conform to schema");
    }

    qDebug() << "XML dictionnary file is consistent with schema";

    m_dico_param_mgr = new MatisseParametersManager();
    m_dico_param_mgr->readDictionnaryFile("config/MatisseParametersDictionnary.xml");

    return true;
}

void MatisseEngine::setJobLauncher(QObject *_job_launcher)
{
    m_job_launcher = _job_launcher;
}

} // namespace matisse
