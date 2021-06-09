#include "process_data_manager.h"

namespace matisse {

const QString ProcessDataManager::RELATIVE_EXECUTION_LOG_PATH = QString("log");

ProcessDataManager* ProcessDataManager::m_temp_instance = NULL;

//// Singleton constructor
ProcessDataManager* ProcessDataManager::instance()
{
    // create instance by lazy initialization
    // guaranteed to be destroyed
    // The instance object is created once and is persistent
    // between 2 invocations of the instance() method
    static ProcessDataManager instance;

    return &instance;
}

//// Constructor for a temporary instance, independant of the singleton instance
ProcessDataManager* ProcessDataManager::newTempInstance()
{
    // Different singleton pattern, guarantees that there is at most a single instance in memory (avoid memory leaks)
    deleteTempInstance();
    m_temp_instance = new ProcessDataManager();

    return m_temp_instance;
}

//// Destructor of tempory instance
void ProcessDataManager::deleteTempInstance()
{
    if (m_temp_instance) {
        delete m_temp_instance;
        m_temp_instance = NULL;
    }
}

ProcessDataManager::ProcessDataManager() :
    m_assemblies_schema(),
    m_assemblies(),
    m_jobs(),
    m_archived_jobs(),
    m_assembly_name_by_file_name(),
    m_job_name_by_file_name(),
    m_archived_job_name_by_file_name(),
    m_archived_jobs_by_assembly()
{

}

ProcessDataManager::~ProcessDataManager()
{

}


void ProcessDataManager::init(QString _data_root_dir, QString _user_data_path)
{
    m_jobs_path = _user_data_path + "/jobs";
    m_jobs_parameters_path = m_jobs_path + "/parameters";
    m_assemblies_path = _user_data_path + "/assemblies";
    m_assemblies_parameters_path = m_assemblies_path + "/parameters";
    //_schemaPath = userDataPath + QDir::separator() + "models";
    m_schema_path = "schemas";

    QDir root(_data_root_dir);
    QString relative_assemblies_path = root.relativeFilePath(m_assemblies_path);
    QString relative_job_path = root.relativeFilePath(m_jobs_path);

    m_assembly_file_pattern = relative_assemblies_path + "/(\\w+\\.xml)";
    m_assembly_parameters_file_pattern = relative_assemblies_path + "/parameters/\\w+\\.xml";
    m_job_file_pattern = relative_job_path + "/(\\w+\\.xml)";
    m_job_parameters_file_pattern = relative_job_path + "/parameters/\\w+\\.xml";
}

bool ProcessDataManager::loadAssembliesAndJobs()
{
    /* Load assemblies */
    clearAssemblies();

    qDebug() << "Loading assemblies...";

    QDir assemblies_dir(m_assemblies_path);
    if (!assemblies_dir.exists()) {
        qCritical() << QString("The assemblies directory '%1' does not exist. Could not load assemblies").arg(m_assemblies_path);
        return false;
    }

    bool read_one = false;

    QStringList assemblies_files = assemblies_dir.entryList(QStringList() << "*.xml");
    foreach(QString assembly_file, assemblies_files) {
        if (!readAssemblyFile(assembly_file)) {
            qWarning() << "Unable to read assembly file " << assembly_file;
            continue;
        }

        read_one = true;
    }

    if (!assemblies_files.isEmpty() && !read_one) {
      /* Assemblies directory populated but could not read any assembly file */
      return false;
    }

    /* Load jobs */
    clearJobs();

    qDebug() << "Loading jobs...";

    read_one = false;

    QDir jobs_dir(m_jobs_path);
    QStringList job_files  = jobs_dir.entryList(QStringList() << "*.xml");
    foreach(QString job_file, job_files) {
        if (!readJobFile(job_file)) {
            qWarning() << "Unable to read job file " << job_file;
            continue;
        }

        read_one = true;
    }

    if (!job_files.isEmpty() && !read_one) {
      /* Jobs directory populated but could not read any job file */
      return false;
    }

    return true;
}

void ProcessDataManager::loadArchivedJobs(QString _archive_path)
{
    clearArchivedJobs();

    qDebug() << "Loading archived jobs";

    QDir archive_dir(_archive_path);
    if (!archive_dir.exists()) {
        qWarning() << QString("The path for archived jobs '%1' does not exist, could not load archived jobs").arg(_archive_path);
        return;
    }

    QStringList archived_job_files = archive_dir.entryList(QStringList() << "*.xml");
    foreach (QString archived_job_file, archived_job_files) {
        if (!readJobFile(archived_job_file, true, _archive_path)) {
            qWarning() << QString("Job file '%1' found in archive path could not be read properly").arg(archived_job_file);
            continue;
        }
    }
}


///
/// \brief ProcessDataManager::readAssemblyFile
/// \param _filename chemin relatif aux assemblages
/// \return
///
bool ProcessDataManager::readAssemblyFile(QString _filename)
{
    qDebug()<< "Reading assembly file "  << _filename;

    if (!m_assemblies_schema.isValid()) {
        loadModels();
    }

    if (!m_assemblies_schema.isValid()) {
        return false;
    }

    QFileInfo file_info(m_assemblies_path + QDir::separator() + _filename);

    if (!xmlIsValid(m_assemblies_schema, file_info)) {
        return false;
    }

    QFile assembly_file(file_info.absoluteFilePath());
    assembly_file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&assembly_file);

    AssemblyDefinition *new_assembly = NULL;

    bool start_descriptor = false;
    bool end_descriptor = false;

    while(!reader.atEnd()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
           continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            QString element_name = reader.name().toString();

            if ("MatisseAssembly" ==  element_name) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString assembly_name = attributes.value("name").toString();
                if (assembly_name != "") {
                    new_assembly = new AssemblyDefinition();
                    new_assembly->setFilename(_filename);
                    new_assembly->setName(assembly_name);
                    new_assembly->setUsable(QVariant(attributes.value("usable").toString()).toBool());
                }
            }
            else if (("Version" == element_name) && start_descriptor && (!end_descriptor)) {
                if (new_assembly) {
                    new_assembly->setVersion(reader.readElementText().simplified());
                }
            }
            else if (("Date" == element_name) && start_descriptor && (!end_descriptor)) {
                if (new_assembly) {
                    QDate date = QDate::fromString(reader.readElementText().simplified(), "dd/MM/yyyy");
                    new_assembly->setCreationDate(date);
                }
            }
            else if (("Author" == element_name)  && start_descriptor && (!end_descriptor)){
                if (new_assembly) {
                    new_assembly->setAuthor(reader.readElementText().simplified());
                }
            }
            else if (("Comments" == element_name) && start_descriptor && (!end_descriptor)) {
                if (new_assembly) {
                    new_assembly->setComment(reader.readElementText().simplified());
                }
            }
            else if ("Parameters" == element_name) {
                // for backward compatibility
                continue;
            }
            else if ("Source" == element_name) {
                if (new_assembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    SourceDefinition* source_definition = new SourceDefinition(name);
                    new_assembly->setSourceDefinition(source_definition);
                }
            }
            else if ("Processor" == element_name) {
                if (new_assembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    quint32 order = (quint32)QVariant(attributes.value("order").toString()).toInt();
                    ProcessorDefinition* processor_definition = new ProcessorDefinition(name, order);
                    new_assembly->addProcessorDef(processor_definition);
                }
            }
            else if ("Destination" == element_name) {
                if (new_assembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    quint32 order = (quint32)QVariant(attributes.value("order").toString()).toInt();
                    DestinationDefinition* destination_definition = new DestinationDefinition(name, order);
                    new_assembly->setDestinationDefinition(destination_definition);
                }
            }
            else if ("Connection" == element_name) {
                if (new_assembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    quint32 start_order = (quint32)QVariant(attributes.value("startOrder").toString()).toInt();
                    quint32 start_line = (quint32)QVariant(attributes.value("startLine").toString()).toInt();
                    quint32 end_order = (quint32)QVariant(attributes.value("endOrder").toString()).toInt();
                    quint32 end_line = (quint32)QVariant(attributes.value("endLine").toString()).toInt();
                    QRgb color = (QRgb)QVariant(attributes.value("color").toString()).toUInt();
                    ConnectionDefinition* connection_definition = new ConnectionDefinition(start_order, start_line, end_order, end_line, color);
                    new_assembly->addConnectionDef(connection_definition);
                }
            }
            else if ("DescriptorFields" == element_name) {
                if (new_assembly) {
                    start_descriptor = true;
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString element_name = reader.name().toString();
            if ("DescriptorFields" == element_name) {
                start_descriptor = false;
                end_descriptor = true;
            }

        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing assembly file :" << reader.error();
        }
    }

    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();
    assembly_file.close();

    if (new_assembly) {
        m_assemblies.insert(new_assembly->name(), new_assembly);
        m_assembly_name_by_file_name.insert(_filename, new_assembly->name());
        qDebug() << "ASSEMBLY DEFINITION VALIDITY" << new_assembly->checkDefinition();
        return true;
    }
    return false;

}

///
/// \brief ProcessDataManager::readJobFile
/// \param _filename chemin relatif aux assemblages
/// \return
///
bool ProcessDataManager::readJobFile(QString _filename, bool _is_archive_file, QString _archive_path)
{
    QString job_base_path;

    if (_is_archive_file) {
        job_base_path = _archive_path; // archive path was controlled by calling function
    } else {
        job_base_path = m_jobs_path;
    }

    QString job_file_path = job_base_path + QDir::separator() + _filename;

    qDebug()<< "Reading job file "  << job_file_path;

    QFileInfo file_info(job_file_path);

    QFile job_file(file_info.absoluteFilePath());
    job_file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&job_file);

    JobDefinition *new_job = NULL;
    ExecutionDefinition *execution_definition = NULL;
    RemoteJobDefinition* remote_job_definition = NULL;

    while(!reader.atEnd()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
           continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            QString elementName = reader.name().toString();

            if ("MatisseJob" ==  elementName) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString job_name = attributes.value("name").toString();
                QString assembly_name = attributes.value("assembly").toString();
                QString assembly_version = attributes.value("version").toString();
                if (job_name != "") {
                    new_job = new JobDefinition(job_name, assembly_name, assembly_version);
                    new_job->setFilename(_filename);
                }
            }
            else if ("Comments" == elementName) {
                if (new_job) {
                    new_job->setComment(reader.readElementText());
                }
            }
            else if ("Execution" == elementName) {
                if (new_job) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    bool executed = QVariant(attributes.value("executed").toString()).toBool();
                    QString date_str = attributes.value("executionDate").toString();
                    QDateTime date = QDateTime::fromString(date_str, "dd/MM/yyyy HH:mm");
                    execution_definition = new ExecutionDefinition();
                    execution_definition->setExecuted(executed);
                    execution_definition->setExecutionDate(date);
                    new_job->setExecutionDefinition(execution_definition);
                }
            }
            else if ("Result" == elementName) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString filename = attributes.value("filename").toString();
                if (execution_definition) {
                   QStringList results = execution_definition->resultFileNames();

                   results << filename;
                   execution_definition->setResultFileNames(results);
                }
            }
            else if ("Remote" == elementName) {
              if (new_job) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString node = attributes.value("node").toString();
                int job_id = attributes.value("jobId").toString().toInt();
                QString timestamp_str = attributes.value("timestamp").toString();
                QDateTime timestamp = QDateTime::fromString(timestamp_str, "dd/MM/yyyy HH:mm");
                remote_job_definition = new RemoteJobDefinition();
                remote_job_definition->setScheduled(true);
                remote_job_definition->setNode(node);
                remote_job_definition->setJobId(job_id);
                remote_job_definition->setTimestamp(timestamp);
                new_job->setRemoteJobDefinition(remote_job_definition);
              }
            }
        }

        if (reader.hasError()) {
            qWarning() << QString("Parsing error for job definition file").arg(job_file_path);
        }

        if (new_job && !remote_job_definition) {
          /* Initialize remote definition empty */
          remote_job_definition = new RemoteJobDefinition();
          remote_job_definition->setScheduled(false);
          new_job->setRemoteJobDefinition(remote_job_definition);
        }
    }

    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();
    job_file.close();

    if (!new_job) {
        qCritical() << QString("Job file '%1' could not be resolved to job definition").arg(job_file_path);
        return false;
    }

    if (_is_archive_file) {
        m_archived_jobs.insert(new_job->name(), new_job);
        m_archived_job_name_by_file_name.insert(_filename, new_job->name());
        QString assembly_name = new_job->assemblyName();

        QStringList *archived_jobs;
//        if (!_assembliesWithArchivedJobs.contains(assemblyName)) {
//            _assembliesWithArchivedJobs.insert(assemblyName);
//        }

        if (m_archived_jobs_by_assembly.contains(assembly_name)) {
            archived_jobs = m_archived_jobs_by_assembly.value(assembly_name);
        } else {
            archived_jobs = new QStringList();
            m_archived_jobs_by_assembly.insert(assembly_name, archived_jobs);
        }

        archived_jobs->append(new_job->name());

    } else {
        m_jobs.insert(new_job->name(), new_job);
        m_job_name_by_file_name.insert(_filename, new_job->name());
    }

    return true;
}

bool ProcessDataManager::writeJobFile(JobDefinition * _job, bool _overwrite)
{
    if (!_job) {
        qCritical() << "Cannot write null job";
        return false;
    }

    QString filename = _job->filename();
    if (filename.isEmpty()) {
        filename = fromNameToFileName(_job->name());
        _job->setFilename(filename);
    }

    QString job_file_full_path = getJobsBasePath() + QDir::separator() + filename;
    qDebug() << "Job file full path : " + job_file_full_path;

    QFileInfo fi(job_file_full_path);
    qDebug() << "Job file absolute path : " + fi.absolutePath();
    QFile job_file(job_file_full_path);

    if (job_file.exists() && (!_overwrite)) {
        return false;
    }

    if (!job_file.open(QFile::WriteOnly)) {
        qCritical() << QString("Error while opening file '%1'").arg(job_file_full_path);
        return false;
    }

    QXmlStreamWriter xsw(&job_file);
    xsw.setAutoFormatting(true);
    xsw.writeStartDocument();

    xsw.writeStartElement("MatisseJob");
    QXmlStreamAttribute jobNameAttr("name", _job->name());
    xsw.writeAttribute(jobNameAttr);
    QXmlStreamAttribute assemblyNameAttr("assembly", _job->assemblyName());
    xsw.writeAttribute(assemblyNameAttr);
    QXmlStreamAttribute versionAttr("version", _job->assemblyVersion());
    xsw.writeAttribute(versionAttr);

    xsw.writeStartElement("Comments");
    xsw.writeCharacters(_job->comment());
    xsw.writeEndElement();

    xsw.writeStartElement("Execution");
    QXmlStreamAttribute executed_attr("executed", QVariant(_job->executionDefinition()->executed()).toString());
    xsw.writeAttribute(executed_attr);

    if (_job->executionDefinition()->executed()) {
        QString job_execution_date = _job->executionDefinition()->executionDate().toString("dd/MM/yyyy hh:mm");
        QXmlStreamAttribute execution_date_attr("executionDate", job_execution_date);
        xsw.writeAttribute(execution_date_attr);

        foreach (QString result_file_name, _job->executionDefinition()->resultFileNames()) {
            xsw.writeStartElement("Result");
            QXmlStreamAttribute filename_attr("filename", result_file_name);
            xsw.writeAttribute(filename_attr);
            xsw.writeEndElement();
        }
    } else {
        QXmlStreamAttribute execution_date("executionDate", "");
        xsw.writeAttribute(execution_date);
    }

    xsw.writeEndElement(); // Execution

    if (_job->remoteJobDefinition()->isScheduled()) {
      xsw.writeStartElement("Remote");
      
      QXmlStreamAttribute node_attr("node", _job->remoteJobDefinition()->node());
      xsw.writeAttribute(node_attr);
      QXmlStreamAttribute job_id_attr("jobId", QString::number(_job->remoteJobDefinition()->jobId()));
      xsw.writeAttribute(job_id_attr);
      QString job_schedule_timestamp = _job->remoteJobDefinition()->timestamp().toString("dd/MM/yyyy hh:mm");
      QXmlStreamAttribute timestamp_attr("timestamp", job_schedule_timestamp);
      xsw.writeAttribute(timestamp_attr);

      xsw.writeEndElement(); // Remote
    }

    xsw.writeEndElement(); // MatisseJob

    if (xsw.hasError()) {
        qCritical() << "Errors occurred while writing XML job file :";
        return false;
    }

    job_file.close();

    return true;
}


bool ProcessDataManager::writeAssemblyFile(AssemblyDefinition * _assembly, bool _overwrite)
{
    if (!_assembly) {
        qCritical() << "Cannot write null assembly";
        return false;
    }

    QString filename = _assembly->filename();
    if (filename.isEmpty()) {
        filename = fromNameToFileName(_assembly->name());
        _assembly->setFilename(filename);
    }

    QString assembly_file_full_path = m_assemblies_path + QDir::separator() + filename;

    QFile assembly_file(assembly_file_full_path);

    if (assembly_file.exists() && (!_overwrite)) {
        return false;
    }

    if (!assembly_file.open(QFile::WriteOnly)) {
        qCritical() << QString("Error while opening file '%1'").arg(assembly_file_full_path);
        return false;
    }

    QXmlStreamWriter xsw(&assembly_file);
    xsw.setAutoFormatting(true);
    xsw.writeStartDocument();

    xsw.writeStartElement("MatisseAssembly");
    QXmlStreamAttribute assembly_name_attr("name", _assembly->name());
    xsw.writeAttribute(assembly_name_attr);
    QXmlStreamAttribute assemblyUsableAttr("usable", QVariant(_assembly->usable()).toString());
    xsw.writeAttribute(assemblyUsableAttr);

    xsw.writeStartElement("DescriptorFields");

    xsw.writeStartElement("Author");
    xsw.writeCharacters(_assembly->author());
    xsw.writeEndElement();

    xsw.writeStartElement("Version");
    xsw.writeCharacters(_assembly->version());
    xsw.writeEndElement();

    xsw.writeStartElement("Comments");
    xsw.writeCharacters(_assembly->comment());
    xsw.writeEndElement();

    xsw.writeStartElement("Date");
    xsw.writeCharacters(_assembly->date());
    xsw.writeEndElement();

    xsw.writeEndElement();

    /* Write source definition */
    xsw.writeStartElement("Source");
    QXmlStreamAttribute source_id_attr("id", QString::number(99));
    xsw.writeAttribute(source_id_attr);
    QXmlStreamAttribute source_name_attr("name", _assembly->sourceDefinition()->name());
    xsw.writeAttribute(source_name_attr);
    QXmlStreamAttribute source_order_attr("order", QString::number(_assembly->sourceDefinition()->order()));
    xsw.writeAttribute(source_order_attr);
    xsw.writeEndElement();

    /* Write processors definition */
    xsw.writeStartElement("Processors");

    foreach (ProcessorDefinition *processor, _assembly->processorDefs()) {
        xsw.writeStartElement("Processor");

        QXmlStreamAttribute processor_id_attr("id", QString::number(99));
        xsw.writeAttribute(processor_id_attr);
        QXmlStreamAttribute processor_name_attr("name", processor->name());
        xsw.writeAttribute(processor_name_attr);
        QXmlStreamAttribute processor_order_attr("order", QString::number(processor->order()));
        xsw.writeAttribute(processor_order_attr);

        xsw.writeEndElement();
    }

    xsw.writeEndElement();

    /* Write destination definition */
    xsw.writeStartElement("Destination");
    QXmlStreamAttribute destination_id_attr("id", QString::number(99));
    xsw.writeAttribute(destination_id_attr);
    QXmlStreamAttribute destination_name_attr("name", _assembly->destinationDefinition()->name());
    xsw.writeAttribute(destination_name_attr);
    QXmlStreamAttribute destination_order_attr("order", QString::number(_assembly->destinationDefinition()->order()));
    xsw.writeAttribute(destination_order_attr);
    xsw.writeEndElement();

    /* Write connections definition */
    xsw.writeStartElement("Connections");

    foreach (ConnectionDefinition *connection, _assembly->connectionDefs()) {
        xsw.writeStartElement("Connection");

        QXmlStreamAttribute conn_start_order_attr("startOrder", QString::number(connection->startOrder()));
        xsw.writeAttribute(conn_start_order_attr);
        QXmlStreamAttribute conn_start_line_attr("startLine", QString::number(connection->startLine()));
        xsw.writeAttribute(conn_start_line_attr);
        QXmlStreamAttribute conn_end_order_attr("endOrder", QString::number(connection->endOrder()));
        xsw.writeAttribute(conn_end_order_attr);
        QXmlStreamAttribute conn_end_line_attr("endLine", QString::number(connection->endLine()));
        xsw.writeAttribute(conn_end_line_attr);
        QXmlStreamAttribute conn_color_attr("color", QString("%1").arg(connection->color()));
        xsw.writeAttribute(conn_color_attr);

        xsw.writeEndElement();
    }

    xsw.writeEndElement();


    xsw.writeEndElement(); // MatisseAssembly

    if (xsw.hasError()) {
        qCritical() << "Errors occurred while writing XML assembly file :";
        return false;
    }

    assembly_file.close();

    return true;
}


QStringList ProcessDataManager::getAssembliesList()
{
    return m_assemblies.keys();
}

QString ProcessDataManager::getAssembliesPath()
{
    return m_assemblies_path;
}

AssemblyDefinition *ProcessDataManager::getAssembly(QString _assembly_name)
{
    return m_assemblies.value(_assembly_name, NULL);
}

JobDefinition *ProcessDataManager::getJob(QString _job_name)
{
    return m_jobs.value(_job_name, NULL);
}

JobDefinition *ProcessDataManager::getArchivedJob(QString _archived_job_name)
{
    return m_archived_jobs.value(_archived_job_name, NULL);
}

QStringList ProcessDataManager::getAssemblyArchivedJobs(QString _assembly_name) const
{
    if (!m_archived_jobs_by_assembly.contains(_assembly_name)) {
        qWarning() << QString("No archived jobs for assembly %1").arg(_assembly_name);
        return QStringList();
    }

    QStringList *archived_jobs = m_archived_jobs_by_assembly.value(_assembly_name);

    return *archived_jobs;
}

QStringList ProcessDataManager::getArchivedJobNames() const
{
    QStringList archived_job_names = m_archived_jobs.keys();
    return archived_job_names;
}

bool ProcessDataManager::restoreArchivedJobs(QString _archive_path, QString _assembly_name, QStringList _jobs_to_restore)
{
    QDir archive_dir(_archive_path);
    if (!archive_dir.exists()) {
        qCritical() << QString("Archive directory '%1' does not exist, cannot restore job files").arg(_archive_path);
        return false;
    }

    if (!m_archived_jobs_by_assembly.contains(_assembly_name)) {
        qWarning() << QString("No archived jobs for assembly '%1'. Could not restore jobs").arg(_assembly_name);
        return false;
    }

    QStringList *archived_jobs = m_archived_jobs_by_assembly.value(_assembly_name);

    bool restored_one = false;

    foreach (QString job_to_restore, _jobs_to_restore) {
        if (!archived_jobs->contains(job_to_restore)) {
            qCritical() << QString("Assembly '%1' does not have an archived job named '%2'");
            continue;
        }

        bool restored = restoreArchivedJob(_archive_path, job_to_restore);
        if (restored) {
            archived_jobs->removeOne(job_to_restore); /* for immediate consistency */
            /* memory will be deallocated when reloading assembly tree */
        }

        /* operation is successful if at least one job has been restored */
        restored_one = restored_one || restored;
    }

    return restored_one;
}

bool ProcessDataManager::restoreArchivedJob(QString _archive_path, QString _job_to_restore) {

    /* archive path was checked by calling class */

    qDebug() << QString("Restoring job '%1'...").arg(_job_to_restore);

    if (m_jobs.contains(_job_to_restore)) {
        /* Technically inconsistent case, better eliminate it anyway */
        qCritical() << QString("Job '%1' was found in displayed jobs list, could not restore.").arg(_job_to_restore);
        return false;
    }

    JobDefinition *job = m_archived_jobs.value(_job_to_restore);
    QString job_file_name = job->filename();

    /* Resolving source and target paths */
    QString source_job_file_path = _archive_path + "/" + job_file_name;
    QString target_job_file_path = m_jobs_path + "/" + job_file_name;

    QString job_parameters_base_relative_path = m_jobs_parameters_path;
    job_parameters_base_relative_path.remove(0, m_jobs_path.length() + 1);
    QString source_job_parameters_file_path = _archive_path + "/" + job_parameters_base_relative_path + "/" + job_file_name;
    QString target_job_parameters_file_path = m_jobs_parameters_path + "/" + job_file_name;

    QFile target_job_file(target_job_file_path);
    if (target_job_file.exists()) {
        qWarning() << QString("Found file '%1', removing...").arg(target_job_file_path);
        bool removed = target_job_file.remove();
        if (!removed) {
            qCritical() << QString("Failed to remove file '%1', could not restore job.").arg(target_job_file_path);
            return false;
        }
    }

    QFile target_job_parameters_file(target_job_parameters_file_path);
    if (target_job_parameters_file.exists()) {
        qWarning() << QString("Found file '%1', removing...").arg(target_job_parameters_file_path);
        bool removed = target_job_parameters_file.remove();
        if (!removed) {
            qCritical() << QString("Failed to remove file '%1', could not restore job.").arg(target_job_parameters_file_path);
            return false;
        }
    }

    bool moved_job_file = QFile::rename(source_job_file_path, target_job_file_path);

    if (moved_job_file) {
        qDebug() << QString("Moved file %1 to %2").arg(source_job_file_path).arg(target_job_file_path);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(source_job_file_path).arg(target_job_file_path);
        return false;
    }

    bool moved_job_parameters_file = QFile::rename(source_job_parameters_file_path, target_job_parameters_file_path);

    if (moved_job_parameters_file) {
        qDebug() << QString("Moved file %1 to %2").arg(source_job_parameters_file_path).arg(target_job_parameters_file_path);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(source_job_parameters_file_path).arg(target_job_parameters_file_path);
        return false;
    }

    return true;
}





QStringList ProcessDataManager::getJobsNames()
{
    return m_jobs.keys();
}

QString ProcessDataManager::getJobsBasePath()
{
    return m_jobs_path;
}

QString ProcessDataManager::getJobParametersFilePath(QString _job_name)
{
    return QDir::cleanPath( m_jobs_parameters_path
                            + QDir::separator()
                            + _job_name + ".xml"
                            );
}

bool ProcessDataManager::loadModels()
{

    QFile assemblies_xsd(m_schema_path + "/MatisseAssembly.xsd");

    if (!assemblies_xsd.exists()) {
        qCritical() << "Error finding" << assemblies_xsd.fileName();
        return false;
    }

    if (!assemblies_xsd.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening MatisseAssembly.xsd";
        return false;
    }

    if (!m_assemblies_schema.load(&assemblies_xsd, QUrl::fromLocalFile(assemblies_xsd.fileName()))) {
        qCritical() << "Error loading MatisseAssembly.xsd";
        return false;
    }

    if (!m_assemblies_schema.isValid()) {
        qCritical() << "Error MatisseAssemblies.xsd is not valid";
        return false;
    }

    assemblies_xsd.close();

    return true;

}

bool ProcessDataManager::xmlIsValid(QXmlSchema &_schema, QFileInfo _file_info)
{

    if (!_schema.isValid()) {
        qCritical() << "Error validation schema invalid";
        return false;
    }

    QFile file(_file_info.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening file" << QFileInfo(file).absoluteFilePath() ;
        return false;
    }

    QXmlSchemaValidator validator(_schema);
    try {
        if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName()))) {
            qCritical() << "Error:" <<  QFileInfo(file).absoluteFilePath()  << "is not compliant with the schema";
            file.close();
            return false;
        }
    } catch (std::exception &excep) {
        qCritical() << "Erreur fichier" << file.fileName();
        qCritical() << "Exception:" << excep.what();
    }

    file.close();
    return true;
}

bool ProcessDataManager::duplicateElementParameters(QString _filename, QString _new_filename, bool _is_assembly)
{
    QString parameters_base_path;

    if (_is_assembly) {
        parameters_base_path = m_assemblies_parameters_path;
    } else {
        parameters_base_path = m_jobs_parameters_path;
    }

    QString parameters_file_path = parameters_base_path + "/" + _filename;
    QFile element_parameters_file(parameters_file_path);

    if (!element_parameters_file.exists()) {
        qCritical() << QString("Could not find parameters file '%1'. Failed to duplicate parameters").arg(_filename);
        return false;
    }

    QString target_parameters_file_path = parameters_base_path + "/" + _new_filename;
    QFile target_parameters_file(target_parameters_file_path);

    if (target_parameters_file.exists()) {
        qCritical() << QString("Target parameters file '%1' already exists. Failed to duplicate parameters").arg(target_parameters_file_path);
        return false;
    }

    bool copied = element_parameters_file.copy(target_parameters_file_path);

    if (copied) {
        qDebug() << QString("Copied parameters file to '%1'").arg(target_parameters_file_path);
        return true;
    } else {
        qCritical() << QString("Failed to copy parameters file '%1' to '%2'").arg(parameters_file_path).arg(target_parameters_file_path);
        return true;
    }
}

QString ProcessDataManager::getJobsParametersBasePath() const
{
    return m_jobs_parameters_path;
}

QString ProcessDataManager::getJobParametersFilePattern() const
{
    return m_job_parameters_file_pattern;
}

QString ProcessDataManager::getAssemblyNameByFilePath(QString _filename) const
{
    QFileInfo file_info(_filename);

    if (!file_info.exists()) {
        qCritical() << QString("File '%1' does not exist, impossible to resove assembly name.").arg(_filename);
        return "";
    }

    QString file_name = file_info.fileName();

    QString assembly_name;

    if (m_assembly_name_by_file_name.contains(file_name)) {
        assembly_name = m_assembly_name_by_file_name.value(file_name);
    } else {
        qWarning() << QString("No assembly found in local repository for file name '%1'").arg(file_name);
        assembly_name = file_name; // using file name instead to designate the assembly
        assembly_name.chop(4); // remove '.xml' suffix
    }

    return assembly_name;
}

QString ProcessDataManager::getJobNameByFilePath(QString _filename) const
{
    QFileInfo file_info(_filename);

    if (!file_info.exists()) {
        qCritical() << QString("File '%1' does not exist, impossible to resove job name.").arg(_filename);
        return "";
    }

    QString file_name = file_info.fileName();

    QString job_name;

    if (m_job_name_by_file_name.contains(file_name)) {
        job_name = m_job_name_by_file_name.value(file_name);
    } else {
        qWarning() << QString("No job found in local repository for file name '%1'").arg(file_name);
        job_name = file_name; // using file name instead to designate the job
        job_name.chop(4); // remove '.xml' suffix
    }

    return job_name;
}

QString ProcessDataManager::getJobFilePattern() const
{
    return m_job_file_pattern;
}

QString ProcessDataManager::getAssemblyParametersFilePattern() const
{
    return m_assembly_parameters_file_pattern;
}

QString ProcessDataManager::getAssemblyFilePattern() const
{
    return m_assembly_file_pattern;
}

QString ProcessDataManager::getAssembliesParametersPath() const
{
    return m_assemblies_parameters_path;
}



void ProcessDataManager::clearAssemblies()
{
    m_assemblies.clear();
    m_assembly_name_by_file_name.clear();
}

void ProcessDataManager::clearJobs()
{
    m_jobs.clear();
    m_job_name_by_file_name.clear();
}

void ProcessDataManager::clearArchivedJobs()
{
    m_archived_jobs.clear();
    m_archived_job_name_by_file_name.clear();

    /* deallocate memory for archived jobs collections */
    QList<QString> keys = m_archived_jobs_by_assembly.keys();
    foreach (QString key, keys) {
        QStringList *archived_jobs = m_archived_jobs_by_assembly.value(key);
        m_archived_jobs_by_assembly.remove(key);
        delete archived_jobs;
    }

    //_assembliesWithArchivedJobs.clear();
}

bool ProcessDataManager::copyJobFilesToResult(QString _job_name, QString _result_path)
{
    qDebug() << "Copying job files to result directory...";

    if (!m_jobs.contains(_job_name)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot copy files to result dir").arg(_job_name);
        return false;
    }

    JobDefinition *current_job = m_jobs.value(_job_name);
    QString job_file_name = current_job->filename();

    QDir result_dir(_result_path);
    if (!result_dir.exists()) {
        qCritical() << QString("Result dir '%1' does not exist, impossible to copy job XML files").arg(_result_path);
        return false;
    }

    /* Creating log path if necessary */
    QString job_log_dir_path = _result_path + "/" + RELATIVE_EXECUTION_LOG_PATH;
    QDir job_log_dir(job_log_dir_path);
    if (!job_log_dir.exists()) {
        qDebug() << "Creating job logs dir...";
        bool created = job_log_dir.mkdir(".");

        if (!created) {
            qCritical() << QString("Failed to create job logs dir '%1', could not copy job files").arg(job_log_dir_path);
            return false;
        }
    }

    /* Resolving source and target paths */
    QString source_job_file_path = m_jobs_path + "/" + job_file_name;
    QString target_job_file_path = job_log_dir_path + "/" + job_file_name;

    QString source_job_parameters_file_path = m_jobs_parameters_path + "/" + job_file_name;

    QString job_parameters_base_relative_path = m_jobs_parameters_path;
    job_parameters_base_relative_path.remove(0, m_jobs_path.length() + 1);

    QString target_job_parameters_base_path = job_log_dir_path + "/" + job_parameters_base_relative_path;
    QDir target_job_parameters_base_dir(target_job_parameters_base_path);
    target_job_parameters_base_dir.mkdir(".");
    QString target_job_parameters_file_path = target_job_parameters_base_path + "/" + job_file_name;

    /* Removing existing copy files */
    bool copy_job_file = true;
    QFile previous_job_file(target_job_file_path);
    if (previous_job_file.exists()) {
        bool removed = previous_job_file.remove();
        if (!removed) {
            qCritical() << QString("Could not remove previous job file copy '%1").arg(target_job_file_path);
            copy_job_file = false;
        }
    }

    bool copy_job_parameters_file = true;
    QFile previous_job_parameters_file(target_job_parameters_file_path);
    if (previous_job_parameters_file.exists()) {
        bool removed = previous_job_parameters_file.remove();
        if (!removed) {
            qCritical() << QString("Could not remove previous job parameters file copy '%1").arg(target_job_parameters_file_path);
            copy_job_parameters_file = false;
        }
    }

    /* Copy files */
    if (copy_job_file) {
        QFile job_file(source_job_file_path);
        bool job_copied = job_file.copy(target_job_file_path);
        if (job_copied) {
            qDebug() << QString("Copied job file '%1' to result dir").arg(source_job_file_path);
        } else {
            qCritical() << QString("Could not copy job file '%1' to result dir").arg(source_job_file_path);
        }
    }

    if (copy_job_parameters_file) {
        QFile job_parameters_file(source_job_parameters_file_path);
        bool job_parameters_copied = job_parameters_file.copy(target_job_parameters_file_path);
        if (job_parameters_copied) {
            qDebug() << QString("Copied job parameters file '%1' to result dir").arg(source_job_parameters_file_path);
        } else {
            qCritical() << QString("Could not copy job parameters file '%1' to result dir").arg(source_job_parameters_file_path);
        }

        return false;
    }

    return true;
}

bool ProcessDataManager::archiveJobFiles(QString _job_name, QString _archive_path)
{
    qDebug() << "Archiving job...";

    if (!m_jobs.contains(_job_name)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot archive files").arg(_job_name);
        return false;
    }

    QDir archive_dir(_archive_path);
    if (!archive_dir.exists()) {
        qCritical() << QString("Archive directory '%1' does not exist, cannot archive job files").arg(_archive_path);
        return false;
    }

    JobDefinition *current_job = m_jobs.value(_job_name);
    QString job_file_name = current_job->filename();

    /* Resolving source and target paths */
    QString source_job_file_path = m_jobs_path + "/" + job_file_name;
    QString target_job_file_path = _archive_path + "/" + job_file_name;

    QString source_job_parameters_file_path = m_jobs_parameters_path + "/" + job_file_name;

    QString job_parameters_base_relative_path = m_jobs_parameters_path;
    job_parameters_base_relative_path.remove(0, m_jobs_path.length() + 1);

    QString target_job_parameters_base_path = _archive_path + "/" + job_parameters_base_relative_path;
    QDir target_job_parameters_base_dir(target_job_parameters_base_path);
    if (!target_job_parameters_base_dir.exists()) {
        bool created = target_job_parameters_base_dir.mkdir(".");

        if (!created) {
            qCritical() << QString("Could not create archive job parameters dir");
            return false;
        }
    }
    QString target_job_parameters_file_path = target_job_parameters_base_path + "/" + job_file_name;

    bool moved_job_file = QFile::rename(source_job_file_path, target_job_file_path);

    if (moved_job_file) {
        qDebug() << QString("Moved file %1 to %2").arg(source_job_file_path).arg(target_job_file_path);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(source_job_file_path).arg(target_job_file_path);
    }

    bool moved_job_parameters_file = QFile::rename(source_job_parameters_file_path, target_job_parameters_file_path);

    if (moved_job_parameters_file) {
        qDebug() << QString("Moved file %1 to %2").arg(source_job_parameters_file_path).arg(target_job_parameters_file_path);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(source_job_parameters_file_path).arg(target_job_parameters_file_path);
    }

    /* archived jobs collections will be updated when reloading the assembly tree */

    return true;
}

bool ProcessDataManager::duplicateJob(QString _job_name, QString _new_name)
{
    qDebug() << "Duplicating job...";

    if (!m_jobs.contains(_job_name)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot duplicate").arg(_job_name);
        return false;
    }

    JobDefinition *job = m_jobs.value(_job_name);
    QString new_file_name = fromNameToFileName(_new_name);
    JobDefinition *new_job = job->duplicate(_new_name, new_file_name);
    bool job_file_written = writeJobFile(new_job);

    if (!job_file_written) {
        qCritical() << QString("Failed to duplicate job '%1'").arg(_job_name);
        return false;
    }

    return duplicateElementParameters(job->filename(), new_job->filename(), false);
}

bool ProcessDataManager::duplicateAssembly(QString _assembly_name, QString _new_name)
{
    qDebug() << "Duplicating job...";

    if (!m_assemblies.contains(_assembly_name)) {
        qCritical() << QString("Assembly '%1' not found in local repository, cannot duplicate").arg(_assembly_name);
        return false;
    }

    AssemblyDefinition *assembly = m_assemblies.value(_assembly_name);
    QString new_file_name = fromNameToFileName(_new_name);
    AssemblyDefinition *new_assembly = assembly->duplicate(_new_name, new_file_name);
    bool assembly_file_written = writeAssemblyFile(new_assembly);

    if (!assembly_file_written) {
        qCritical() << QString("Failed to duplicate assembly '%1'").arg(_assembly_name);
        return false;
    }

    return duplicateElementParameters(assembly->filename(), new_file_name, true);
}

QString ProcessDataManager::fromNameToFileName(QString _name)
{
    QString file_name = _name;
    file_name.replace(" ", "_"); // normalize name
    file_name.append(".xml");    // append file extension
    return file_name;
}

bool ProcessDataManager::assemblyHasArchivedJob(QString _assembly_name)
{
    return m_archived_jobs_by_assembly.contains(_assembly_name);
}

} // namespace matisse
