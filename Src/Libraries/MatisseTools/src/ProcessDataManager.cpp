#include "ProcessDataManager.h"

using namespace MatisseTools;

const QString ProcessDataManager::RELATIVE_EXECUTION_LOG_PATH = QString("log");

ProcessDataManager::ProcessDataManager(QString dataRootDir, QString userDataPath)
{
    _jobsPath = userDataPath + "/jobs";
    _jobsParametersPath = _jobsPath + "/parameters";
    _assembliesPath = userDataPath + "/assemblies";
    _assembliesParametersPath = _assembliesPath + "/parameters";
    _schemaPath = userDataPath + QDir::separator() + "models";

    QDir root(dataRootDir);
    QString relativeAssembliesPath = root.relativeFilePath(_assembliesPath);
    QString relativeJobPath = root.relativeFilePath(_jobsPath);

    _assemblyFilePattern = relativeAssembliesPath + "/(\\w+\\.xml)";
    _assemblyParametersFilePattern = relativeAssembliesPath + "/parameters/\\w+\\.xml";
    _jobFilePattern = relativeJobPath + "/(\\w+\\.xml)";
    _jobParametersFilePattern = relativeJobPath + "/parameters/\\w+\\.xml";
}

void ProcessDataManager::loadAssembliesAndJobs()
{
    /* Load assemblies */
    clearAssemblies();

    qDebug() << "Loading assemblies...";

    QDir assembliesDir(_assembliesPath);
    if (!assembliesDir.exists()) {
        qCritical() << QString("The assemblies directory '%1' does not exist. Could not load assemblies").arg(_assembliesPath);
        return;
    }

    QStringList assembliesFiles = assembliesDir.entryList(QStringList() << "*.xml");
    foreach(QString assemblyFile, assembliesFiles) {
        if (!readAssemblyFile(assemblyFile)) {
            qWarning() << "Unable to read assembly file " << assemblyFile;
            continue;
        }
    }

    /* Load jobs */
    clearJobs();

    qDebug() << "Loading jobs...";

    QDir jobsDir(_jobsPath);
    QStringList jobFiles  = jobsDir.entryList(QStringList() << "*.xml");
    foreach(QString jobFile, jobFiles) {
        if (!readJobFile(jobFile)) {
            qWarning() << "Unable to read job file " << jobFile;
            continue;
        }
    }
}

void ProcessDataManager::loadArchivedJobs(QString archivePath)
{
    clearArchivedJobs();

    qDebug() << "Loading archived jobs";

    QDir archiveDir(archivePath);
    if (!archiveDir.exists()) {
        qWarning() << QString("The path for archived jobs '%1' does not exist, could not load archived jobs").arg(archivePath);
        return;
    }

    QStringList archivedJobFiles = archiveDir.entryList(QStringList() << "*.xml");
    foreach (QString archivedJobFile, archivedJobFiles) {
        if (!readJobFile(archivedJobFile, true, archivePath)) {
            qWarning() << QString("Job file '%1' found in archive path could not be read properly").arg(archivedJobFile);
            continue;
        }
    }
}


///
/// \brief ProcessDataManager::readAssemblyFile
/// \param filename chemin relatif aux assemblages
/// \return
///
bool ProcessDataManager::readAssemblyFile(QString filename)
{
    qDebug()<< "Reading assembly file "  << filename;

    if (!_assembliesSchema.isValid()) {
        loadModels();
    }

    if (!_assembliesSchema.isValid()) {
        return false;
    }

    QFileInfo fileInfo(_assembliesPath + QDir::separator() + filename);

    if (!xmlIsValid(_assembliesSchema, fileInfo)) {
        return false;
    }

    QFile assemblyFile(fileInfo.absoluteFilePath());
    assemblyFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&assemblyFile);

    AssemblyDefinition *newAssembly = NULL;

    bool startDescriptor = false;
    bool endDescriptor = false;

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

            if ("MatisseAssembly" ==  elementName) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString assemblyName = attributes.value("name").toString();
                if (assemblyName != "") {
                    newAssembly = new AssemblyDefinition();
                    newAssembly->setFilename(filename);
                    newAssembly->setName(assemblyName);
                    newAssembly->setUsable(QVariant(attributes.value("usable").toString()).toBool());
                    newAssembly->setIsRealTime(QVariant(attributes.value("isRealTime").toString()).toBool());
                }
            }
            else if (("Version" == elementName) && startDescriptor && (!endDescriptor)) {
                if (newAssembly) {
                    newAssembly->setVersion(reader.readElementText().simplified());
                }
            }
            else if (("Date" == elementName) && startDescriptor && (!endDescriptor)) {
                if (newAssembly) {
                    QDate date = QDate::fromString(reader.readElementText().simplified(), "dd/MM/yyyy");
                    newAssembly->setCreationDate(date);
                }
            }
            else if (("Author" == elementName)  && startDescriptor && (!endDescriptor)){
                if (newAssembly) {
                    newAssembly->setAuthor(reader.readElementText().simplified());
                }
            }
            else if (("Comments" == elementName) && startDescriptor && (!endDescriptor)) {
                if (newAssembly) {
                    newAssembly->setComment(reader.readElementText().simplified());
                }
            }
            else if ("Parameters" == elementName) {
                // for backward compatibility
                continue;
            }
            else if ("Source" == elementName) {
                if (newAssembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    SourceDefinition* sourceDefinition = new SourceDefinition(name);
                    newAssembly->setSourceDefinition(sourceDefinition);
                }
            }
            else if ("Processor" == elementName) {
                if (newAssembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    quint32 order = (quint32)QVariant(attributes.value("order").toString()).toInt();
                    ProcessorDefinition* processorDefinition = new ProcessorDefinition(name, order);
                    newAssembly->addProcessorDef(processorDefinition);
                }
            }
            else if ("Destination" == elementName) {
                if (newAssembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString name = attributes.value("name").toString();
                    quint32 order = (quint32)QVariant(attributes.value("order").toString()).toInt();
                    DestinationDefinition* destinationDefinition = new DestinationDefinition(name, order);
                    newAssembly->setDestinationDefinition(destinationDefinition);
                }
            }
            else if ("Connection" == elementName) {
                if (newAssembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    quint32 startOrder = (quint32)QVariant(attributes.value("startOrder").toString()).toInt();
                    quint32 startLine = (quint32)QVariant(attributes.value("startLine").toString()).toInt();
                    quint32 endOrder = (quint32)QVariant(attributes.value("endOrder").toString()).toInt();
                    quint32 endLine = (quint32)QVariant(attributes.value("endLine").toString()).toInt();
                    QRgb color = (QRgb)QVariant(attributes.value("color").toString()).toUInt();
                    ConnectionDefinition* connectionDefinition = new ConnectionDefinition(startOrder, startLine, endOrder, endLine, color);
                    newAssembly->addConnectionDef(connectionDefinition);
                }
            }
            else if ("DescriptorFields" == elementName) {
                if (newAssembly) {
                    startDescriptor = true;
                }
            }
        } else if (token == QXmlStreamReader::EndElement) {
            QString elementName = reader.name().toString();
            if ("DescriptorFields" == elementName) {
                startDescriptor = false;
                endDescriptor = true;
            }

        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing assembly file :" << reader.error();
        }
    }

    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();
    assemblyFile.close();

    if (newAssembly) {
        _assemblies.insert(newAssembly->name(), newAssembly);
        _assemblyNameByFileName.insert(filename, newAssembly->name());
        qDebug() << "ASSEMBLY DEFINITION VALIDITY" << newAssembly->checkDefinition();
        return true;
    }
    return false;

}

///
/// \brief ProcessDataManager::readJobFile
/// \param filename chemin relatif aux assemblages
/// \return
///
bool ProcessDataManager::readJobFile(QString filename, bool isArchiveFile, QString archivePath)
{
    QString jobBasePath;

    if (isArchiveFile) {
        jobBasePath = archivePath; // archive path was controlled by calling function
    } else {
        jobBasePath = _jobsPath;
    }

    QString jobFilePath = jobBasePath + QDir::separator() + filename;

    qDebug()<< "Reading job file "  << jobFilePath;

    QFileInfo fileInfo(jobFilePath);

    QFile jobFile(fileInfo.absoluteFilePath());
    jobFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&jobFile);

    JobDefinition *newJob = NULL;
    ExecutionDefinition *executionDefinition = NULL;

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
                QString jobName = attributes.value("name").toString();
                QString assemblyName = attributes.value("assembly").toString();
                QString assemblyVersion = attributes.value("version").toString();
                if (jobName != "") {
                    newJob = new JobDefinition(jobName, assemblyName, assemblyVersion);
                    newJob->setFilename(filename);
                }
            }
            else if ("Comments" == elementName) {
                if (newJob) {
                    newJob->setComment(reader.readElementText());
                }
            }
            else if ("Execution" == elementName) {
                if (newJob) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    bool executed = QVariant(attributes.value("executed").toString()).toBool();
                    QString dateStr = attributes.value("executionDate").toString();
                    QDateTime date = QDateTime::fromString(dateStr, "dd/MM/yyyy HH:mm");
                    executionDefinition = new ExecutionDefinition();
                    executionDefinition->setExecuted(executed);
                    executionDefinition->setExecutionDate(date);
                    newJob->setExecutionDefinition(executionDefinition);
                }
            }
            else if ("Result" == elementName) {
                QXmlStreamAttributes attributes = reader.attributes();
                QString filename = attributes.value("filename").toString();
                if (executionDefinition) {
                   QStringList results = executionDefinition->resultFileNames();

                   results << filename;
                   executionDefinition->setResultFileNames(results);
                }
            }
        }

        if (reader.hasError()) {
            qWarning() << QString("Parsing error for job definition file").arg(jobFilePath);
        }
    }

    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();
    jobFile.close();

    if (!newJob) {
        qCritical() << QString("Job file '%1' could not be resolved to job definition").arg(jobFilePath);
        return false;
    }

    if (isArchiveFile) {
        _archivedJobs.insert(newJob->name(), newJob);
        _archivedJobNameByFileName.insert(filename, newJob->name());
        QString assemblyName = newJob->assemblyName();

        QStringList *archivedJobs;
//        if (!_assembliesWithArchivedJobs.contains(assemblyName)) {
//            _assembliesWithArchivedJobs.insert(assemblyName);
//        }

        if (_archivedJobsByAssembly.contains(assemblyName)) {
            archivedJobs = _archivedJobsByAssembly.value(assemblyName);
        } else {
            archivedJobs = new QStringList();
            _archivedJobsByAssembly.insert(assemblyName, archivedJobs);
        }

        archivedJobs->append(newJob->name());

    } else {
        _jobs.insert(newJob->name(), newJob);
        _jobNameByFileName.insert(filename, newJob->name());
    }

    return true;
}

bool ProcessDataManager::writeJobFile(JobDefinition * job, bool overWrite)
{
    if (!job) {
        qCritical() << "Cannot write null job";
        return false;
    }

    QString filename = job->filename();
    if (filename.isEmpty()) {
        filename = fromNameToFileName(job->name());
        job->setFilename(filename);
    }

    QString jobFileFullPath = getJobsBasePath() + QDir::separator() + filename;

    QFile jobFile(jobFileFullPath);

    if (jobFile.exists() && (!overWrite)) {
        return false;
    }

    if (!jobFile.open(QFile::WriteOnly)) {
        qCritical() << QString("Error while opening file '%1'").arg(jobFileFullPath);
        return false;
    }

    QXmlStreamWriter xsw(&jobFile);
    xsw.setAutoFormatting(true);
    xsw.writeStartDocument();

    xsw.writeStartElement("MatisseJob");
    QXmlStreamAttribute jobNameAttr("name", job->name());
    xsw.writeAttribute(jobNameAttr);
    QXmlStreamAttribute assemblyNameAttr("assembly", job->assemblyName());
    xsw.writeAttribute(assemblyNameAttr);
    QXmlStreamAttribute versionAttr("version", job->assemblyVersion());
    xsw.writeAttribute(versionAttr);

    xsw.writeStartElement("Comments");
    xsw.writeCharacters(job->comment());
    xsw.writeEndElement();

    xsw.writeStartElement("Execution");
    QXmlStreamAttribute executedAttr("executed", QVariant(job->executionDefinition()->executed()).toString());
    xsw.writeAttribute(executedAttr);

    if (job->executionDefinition()->executed()) {
        QString jobExecutionDate = job->executionDefinition()->executionDate().toString("dd/MM/yyyy hh:mm");
        QXmlStreamAttribute executionDateAttr("executionDate", jobExecutionDate);
        xsw.writeAttribute(executionDateAttr);

        foreach (QString resultFileName, job->executionDefinition()->resultFileNames()) {
            xsw.writeStartElement("Result");
            QXmlStreamAttribute filenameAttr("filename", resultFileName);
            xsw.writeAttribute(filenameAttr);
            xsw.writeEndElement();
        }
    } else {
        QXmlStreamAttribute executionDate("executionDate", "");
        xsw.writeAttribute(executionDate);
    }

    xsw.writeEndElement(); // Execution

    xsw.writeEndElement(); // MatisseJob

    if (xsw.hasError()) {
        qCritical() << "Errors occurred while writing XML job file :";
        return false;
    }

    jobFile.close();

    return true;
}


bool ProcessDataManager::writeAssemblyFile(AssemblyDefinition * assembly, bool overWrite)
{
    if (!assembly) {
        qCritical() << "Cannot write null assembly";
        return false;
    }

    QString filename = assembly->filename();
    if (filename.isEmpty()) {
        filename = fromNameToFileName(assembly->name());
        assembly->setFilename(filename);
    }

    QString assemblyFileFullPath = _assembliesPath + QDir::separator() + filename;

    QFile assemblyFile(assemblyFileFullPath);

    if (assemblyFile.exists() && (!overWrite)) {
        return false;
    }

    if (!assemblyFile.open(QFile::WriteOnly)) {
        qCritical() << QString("Error while opening file '%1'").arg(assemblyFileFullPath);
        return false;
    }

    QXmlStreamWriter xsw(&assemblyFile);
    xsw.setAutoFormatting(true);
    xsw.writeStartDocument();

    xsw.writeStartElement("MatisseAssembly");
    QXmlStreamAttribute assemblyNameAttr("name", assembly->name());
    xsw.writeAttribute(assemblyNameAttr);
    QXmlStreamAttribute assemblyTypeAttr("isRealTime", QVariant(assembly->isRealTime()).toString());
    xsw.writeAttribute(assemblyTypeAttr);
    QXmlStreamAttribute assemblyUsableAttr("usable", QVariant(assembly->usable()).toString());
    xsw.writeAttribute(assemblyUsableAttr);

    xsw.writeStartElement("DescriptorFields");

    xsw.writeStartElement("Author");
    xsw.writeCharacters(assembly->author());
    xsw.writeEndElement();

    xsw.writeStartElement("Version");
    xsw.writeCharacters(assembly->version());
    xsw.writeEndElement();

    xsw.writeStartElement("Comments");
    xsw.writeCharacters(assembly->comment());
    xsw.writeEndElement();

    xsw.writeStartElement("Date");
    xsw.writeCharacters(assembly->date());
    xsw.writeEndElement();

    xsw.writeEndElement();

    /* Write source definition */
    xsw.writeStartElement("Source");
    QXmlStreamAttribute sourceIdAttr("id", QString::number(99));
    xsw.writeAttribute(sourceIdAttr);
    QXmlStreamAttribute sourceNameAttr("name", assembly->sourceDefinition()->name());
    xsw.writeAttribute(sourceNameAttr);
    QXmlStreamAttribute sourceOrderAttr("order", QString::number(assembly->sourceDefinition()->order()));
    xsw.writeAttribute(sourceOrderAttr);
    xsw.writeEndElement();

    /* Write processors definition */
    xsw.writeStartElement("Processors");

    foreach (ProcessorDefinition *processor, assembly->processorDefs()) {
        xsw.writeStartElement("Processor");

        QXmlStreamAttribute processorIdAttr("id", QString::number(99));
        xsw.writeAttribute(processorIdAttr);
        QXmlStreamAttribute processorNameAttr("name", processor->name());
        xsw.writeAttribute(processorNameAttr);
        QXmlStreamAttribute processorOrderAttr("order", QString::number(processor->order()));
        xsw.writeAttribute(processorOrderAttr);

        xsw.writeEndElement();
    }

    xsw.writeEndElement();

    /* Write destination definition */
    xsw.writeStartElement("Destination");
    QXmlStreamAttribute destinationIdAttr("id", QString::number(99));
    xsw.writeAttribute(destinationIdAttr);
    QXmlStreamAttribute destinationNameAttr("name", assembly->destinationDefinition()->name());
    xsw.writeAttribute(destinationNameAttr);
    QXmlStreamAttribute destinationOrderAttr("order", QString::number(assembly->destinationDefinition()->order()));
    xsw.writeAttribute(destinationOrderAttr);
    xsw.writeEndElement();

    /* Write connections definition */
    xsw.writeStartElement("Connections");

    foreach (ConnectionDefinition *connection, assembly->connectionDefs()) {
        xsw.writeStartElement("Connection");

        QXmlStreamAttribute connStartOrderAttr("startOrder", QString::number(connection->startOrder()));
        xsw.writeAttribute(connStartOrderAttr);
        QXmlStreamAttribute connStartLineAttr("startLine", QString::number(connection->startLine()));
        xsw.writeAttribute(connStartLineAttr);
        QXmlStreamAttribute connEndOrderAttr("endOrder", QString::number(connection->endOrder()));
        xsw.writeAttribute(connEndOrderAttr);
        QXmlStreamAttribute connEndLineAttr("endLine", QString::number(connection->endLine()));
        xsw.writeAttribute(connEndLineAttr);
        QXmlStreamAttribute connColorAttr("color", QString("%1").arg(connection->color()));
        xsw.writeAttribute(connColorAttr);

        xsw.writeEndElement();
    }

    xsw.writeEndElement();


    xsw.writeEndElement(); // MatisseAssembly

    if (xsw.hasError()) {
        qCritical() << "Errors occurred while writing XML assembly file :";
        return false;
    }

    assemblyFile.close();

    return true;
}


QStringList ProcessDataManager::getAssembliesList()
{
    return _assemblies.keys();
}

QString ProcessDataManager::getAssembliesPath()
{
    return _assembliesPath;
}

AssemblyDefinition *ProcessDataManager::getAssembly(QString assemblyName)
{
    return _assemblies.value(assemblyName, NULL);
}

JobDefinition *ProcessDataManager::getJob(QString jobName)
{
    return _jobs.value(jobName, NULL);
}

JobDefinition *ProcessDataManager::getArchivedJob(QString archivedJobName)
{
    return _archivedJobs.value(archivedJobName, NULL);
}

QStringList ProcessDataManager::getAssemblyArchivedJobs(QString assemblyName) const
{
    if (!_archivedJobsByAssembly.contains(assemblyName)) {
        qWarning() << QString("No archived jobs for assembly %1").arg(assemblyName);
        return QStringList();
    }

    QStringList *archivedJobs = _archivedJobsByAssembly.value(assemblyName);

    return *archivedJobs;
}

QStringList ProcessDataManager::getArchivedJobNames() const
{
    QStringList archivedJobNames = _archivedJobs.keys();
    return archivedJobNames;
}

bool ProcessDataManager::restoreArchivedJobs(QString archivePath, QString assemblyName, QStringList jobsToRestore)
{
    QDir archiveDir(archivePath);
    if (!archiveDir.exists()) {
        qCritical() << QString("Archive directory '%1' does not exist, cannot restore job files").arg(archivePath);
        return false;
    }

    if (!_archivedJobsByAssembly.contains(assemblyName)) {
        qWarning() << QString("No archived jobs for assembly '%1'. Could not restore jobs").arg(assemblyName);
        return false;
    }

    QStringList *archivedJobs = _archivedJobsByAssembly.value(assemblyName);

    bool restoredOne = false;

    foreach (QString jobToRestore, jobsToRestore) {
        if (!archivedJobs->contains(jobToRestore)) {
            qCritical() << QString("Assembly '%1' does not have an archived job named '%2'");
            continue;
        }

        bool restored = restoreArchivedJob(archivePath, jobToRestore);
        if (restored) {
            archivedJobs->removeOne(jobToRestore); /* for immediate consistency */
            /* memory will be deallocated when reloading assembly tree */
        }

        /* operation is successful if at least one job has been restored */
        restoredOne = restoredOne || restored;
    }

    return restoredOne;
}

bool ProcessDataManager::restoreArchivedJob(QString archivePath, QString jobToRestore) {

    /* archive path was checked by calling class */

    qDebug() << QString("Restoring job '%1'...").arg(jobToRestore);

    if (_jobs.contains(jobToRestore)) {
        /* Technically inconsistent case, better eliminate it anyway */
        qCritical() << QString("Job '%1' was found in displayed jobs list, could not restore.").arg(jobToRestore);
        return false;
    }

    JobDefinition *job = _archivedJobs.value(jobToRestore);
    QString jobFileName = job->filename();

    /* Resolving source and target paths */
    QString sourceJobFilePath = archivePath + "/" + jobFileName;
    QString targetJobFilePath = _jobsPath + "/" + jobFileName;

    QString jobParametersBaseRelativePath = _jobsParametersPath;
    jobParametersBaseRelativePath.remove(0, _jobsPath.length() + 1);
    QString sourceJobParametersFilePath = archivePath + "/" + jobParametersBaseRelativePath + "/" + jobFileName;
    QString targetJobParametersFilePath = _jobsParametersPath + "/" + jobFileName;

    QFile targetJobFile(targetJobFilePath);
    if (targetJobFile.exists()) {
        qWarning() << QString("Found file '%1', removing...").arg(targetJobFilePath);
        bool removed = targetJobFile.remove();
        if (!removed) {
            qCritical() << QString("Failed to remove file '%1', could not restore job.").arg(targetJobFilePath);
            return false;
        }
    }

    QFile targetJobParametersFile(targetJobParametersFilePath);
    if (targetJobParametersFile.exists()) {
        qWarning() << QString("Found file '%1', removing...").arg(targetJobParametersFilePath);
        bool removed = targetJobParametersFile.remove();
        if (!removed) {
            qCritical() << QString("Failed to remove file '%1', could not restore job.").arg(targetJobParametersFilePath);
            return false;
        }
    }

    bool movedJobFile = QFile::rename(sourceJobFilePath, targetJobFilePath);

    if (movedJobFile) {
        qDebug() << QString("Moved file %1 to %2").arg(sourceJobFilePath).arg(targetJobFilePath);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(sourceJobFilePath).arg(targetJobFilePath);
        return false;
    }

    bool movedJobParametersFile = QFile::rename(sourceJobParametersFilePath, targetJobParametersFilePath);

    if (movedJobParametersFile) {
        qDebug() << QString("Moved file %1 to %2").arg(sourceJobParametersFilePath).arg(targetJobParametersFilePath);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(sourceJobParametersFilePath).arg(targetJobParametersFilePath);
        return false;
    }

    return true;
}





QStringList ProcessDataManager::getJobsNames()
{
    return _jobs.keys();
}

QString ProcessDataManager::getJobsBasePath()
{
    return _jobsPath;
}

QString ProcessDataManager::getJobParametersFilePath(QString jobName)
{
    return QDir::cleanPath( _jobsParametersPath
                            + QDir::separator()
                            + jobName + ".xml"
                            );
}

bool ProcessDataManager::loadModels()
{

    QFile assembliesXsd(_schemaPath + "/MatisseAssembly.xsd");

    if (!assembliesXsd.exists()) {
        qCritical() << "Error finding" << assembliesXsd.fileName();
        return false;
    }

    if (!assembliesXsd.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening MatisseAssembly.xsd";
        return false;
    }

    if (!_assembliesSchema.load(&assembliesXsd, QUrl::fromLocalFile(assembliesXsd.fileName()))) {
        qCritical() << "Error loading MatisseAssembly.xsd";
        return false;
    }

    if (!_assembliesSchema.isValid()) {
        qCritical() << "Error MatisseAssemblies.xsd is not valid";
        return false;
    }

    assembliesXsd.close();

    return true;

}

bool ProcessDataManager::xmlIsValid(QXmlSchema &schema, QFileInfo fileInfo)
{

    if (!schema.isValid()) {
        qCritical() << "Error validation schema invalid";
        return false;
    }

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening file" << QFileInfo(file).absoluteFilePath() ;
        return false;
    }

    QXmlSchemaValidator validator(schema);
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

bool ProcessDataManager::duplicateElementParameters(QString fileName, QString newFileName, bool isAssembly)
{
    QString parametersBasePath;

    if (isAssembly) {
        parametersBasePath = _assembliesParametersPath;
    } else {
        parametersBasePath = _jobsParametersPath;
    }

    QString parametersFilePath = parametersBasePath + "/" + fileName;
    QFile elementParametersFile(parametersFilePath);

    if (!elementParametersFile.exists()) {
        qCritical() << QString("Could not find parameters file '%1'. Failed to duplicate parameters").arg(fileName);
        return false;
    }

    QString targetParametersFilePath = parametersBasePath + "/" + newFileName;
    QFile targetParametersFile(targetParametersFilePath);

    if (targetParametersFile.exists()) {
        qCritical() << QString("Target parameters file '%1' already exists. Failed to duplicate parameters").arg(targetParametersFilePath);
        return false;
    }

    bool copied = elementParametersFile.copy(targetParametersFilePath);

    if (copied) {
        qDebug() << QString("Copied parameters file to '%1'").arg(targetParametersFilePath);
        return true;
    } else {
        qCritical() << QString("Failed to copy parameters file '%1' to '%2'").arg(parametersFilePath).arg(targetParametersFilePath);
        return true;
    }
}

QString ProcessDataManager::getJobsParametersBasePath() const
{
    return _jobsParametersPath;
}

QString ProcessDataManager::getJobParametersFilePattern() const
{
    return _jobParametersFilePattern;
}

QString ProcessDataManager::getAssemblyNameByFilePath(QString filePath) const
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        qCritical() << QString("File '%1' does not exist, impossible to resove assembly name.").arg(filePath);
        return "";
    }

    QString fileName = fileInfo.fileName();

    QString assemblyName;

    if (_assemblyNameByFileName.contains(fileName)) {
        assemblyName = _assemblyNameByFileName.value(fileName);
    } else {
        qWarning() << QString("No assembly found in local repository for file name '%1'").arg(fileName);
        assemblyName = fileName; // using file name instead to designate the assembly
        assemblyName.chop(4); // remove '.xml' suffix
    }

    return assemblyName;
}

QString ProcessDataManager::getJobNameByFilePath(QString filePath) const
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists()) {
        qCritical() << QString("File '%1' does not exist, impossible to resove job name.").arg(filePath);
        return "";
    }

    QString fileName = fileInfo.fileName();

    QString jobName;

    if (_jobNameByFileName.contains(fileName)) {
        jobName = _jobNameByFileName.value(fileName);
    } else {
        qWarning() << QString("No job found in local repository for file name '%1'").arg(fileName);
        jobName = fileName; // using file name instead to designate the job
        jobName.chop(4); // remove '.xml' suffix
    }

    return jobName;
}

QString ProcessDataManager::getJobFilePattern() const
{
    return _jobFilePattern;
}

QString ProcessDataManager::getAssemblyParametersFilePattern() const
{
    return _assemblyParametersFilePattern;
}

QString ProcessDataManager::getAssemblyFilePattern() const
{
    return _assemblyFilePattern;
}

QString ProcessDataManager::getAssembliesParametersPath() const
{
    return _assembliesParametersPath;
}



void ProcessDataManager::clearAssemblies()
{
    _assemblies.clear();
    _assemblyNameByFileName.clear();
}

void ProcessDataManager::clearJobs()
{
    _jobs.clear();
    _jobNameByFileName.clear();
}

void ProcessDataManager::clearArchivedJobs()
{
    _archivedJobs.clear();
    _archivedJobNameByFileName.clear();

    /* deallocate memory for archived jobs collections */
    QList<QString> keys = _archivedJobsByAssembly.keys();
    foreach (QString key, keys) {
        QStringList *archivedJobs = _archivedJobsByAssembly.value(key);
        _archivedJobsByAssembly.remove(key);
        delete archivedJobs;
    }

    //_assembliesWithArchivedJobs.clear();
}

bool ProcessDataManager::copyJobFilesToResult(QString jobName, QString resultPath)
{
    qDebug() << "Copying job files to result directory...";

    if (!_jobs.contains(jobName)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot copy files to result dir").arg(jobName);
        return false;
    }

    JobDefinition *currentJob = _jobs.value(jobName);
    QString jobFileName = currentJob->filename();

    QDir resultDir(resultPath);
    if (!resultDir.exists()) {
        qCritical() << QString("Result dir '%1' does not exist, impossible to copy job XML files").arg(resultPath);
        return false;
    }

    /* Creating log path if necessary */
    QString jobLogDirPath = resultPath + "/" + RELATIVE_EXECUTION_LOG_PATH;
    QDir jobLogDir(jobLogDirPath);
    if (!jobLogDir.exists()) {
        qDebug() << "Creating job logs dir...";
        bool created = jobLogDir.mkdir(".");

        if (!created) {
            qCritical() << QString("Failed to create job logs dir '%1', could not copy job files").arg(jobLogDirPath);
            return false;
        }
    }

    /* Resolving source and target paths */
    QString sourceJobFilePath = _jobsPath + "/" + jobFileName;
    QString targetJobFilePath = jobLogDirPath + "/" + jobFileName;

    QString sourceJobParametersFilePath = _jobsParametersPath + "/" + jobFileName;

    QString jobParametersBaseRelativePath = _jobsParametersPath;
    jobParametersBaseRelativePath.remove(0, _jobsPath.length() + 1);

    QString targetJobParametersBasePath = jobLogDirPath + "/" + jobParametersBaseRelativePath;
    QDir targetJobParametersBaseDir(targetJobParametersBasePath);
    targetJobParametersBaseDir.mkdir(".");
    QString targetJobParametersFilePath = targetJobParametersBasePath + "/" + jobFileName;

    /* Removing existing copy files */
    bool copyJobFile = true;
    QFile previousJobFile(targetJobFilePath);
    if (previousJobFile.exists()) {
        bool removed = previousJobFile.remove();
        if (!removed) {
            qCritical() << QString("Could not remove previous job file copy '%1").arg(targetJobFilePath);
            copyJobFile = false;
        }
    }

    bool copyJobParametersFile = true;
    QFile previousJobParametersFile(targetJobParametersFilePath);
    if (previousJobParametersFile.exists()) {
        bool removed = previousJobParametersFile.remove();
        if (!removed) {
            qCritical() << QString("Could not remove previous job parameters file copy '%1").arg(targetJobParametersFilePath);
            copyJobParametersFile = false;
        }
    }

    /* Copy files */
    if (copyJobFile) {
        QFile jobFile(sourceJobFilePath);
        bool jobCopied = jobFile.copy(targetJobFilePath);
        if (jobCopied) {
            qDebug() << QString("Copied job file '%1' to result dir").arg(sourceJobFilePath);
        } else {
            qCritical() << QString("Could not copy job file '%1' to result dir").arg(sourceJobFilePath);
        }
    }

    if (copyJobParametersFile) {
        QFile jobParametersFile(sourceJobParametersFilePath);
        bool jobParametersCopied = jobParametersFile.copy(targetJobParametersFilePath);
        if (jobParametersCopied) {
            qDebug() << QString("Copied job parameters file '%1' to result dir").arg(sourceJobParametersFilePath);
        } else {
            qCritical() << QString("Could not copy job parameters file '%1' to result dir").arg(sourceJobParametersFilePath);
        }

        return false;
    }

    return true;
}

bool ProcessDataManager::archiveJobFiles(QString jobName, QString archivePath)
{
    qDebug() << "Archiving job...";

    if (!_jobs.contains(jobName)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot archive files").arg(jobName);
        return false;
    }

    QDir archiveDir(archivePath);
    if (!archiveDir.exists()) {
        qCritical() << QString("Archive directory '%1' does not exist, cannot archive job files").arg(archivePath);
        return false;
    }

    JobDefinition *currentJob = _jobs.value(jobName);
    QString jobFileName = currentJob->filename();

    /* Resolving source and target paths */
    QString sourceJobFilePath = _jobsPath + "/" + jobFileName;
    QString targetJobFilePath = archivePath + "/" + jobFileName;

    QString sourceJobParametersFilePath = _jobsParametersPath + "/" + jobFileName;

    QString jobParametersBaseRelativePath = _jobsParametersPath;
    jobParametersBaseRelativePath.remove(0, _jobsPath.length() + 1);

    QString targetJobParametersBasePath = archivePath + "/" + jobParametersBaseRelativePath;
    QDir targetJobParametersBaseDir(targetJobParametersBasePath);
    if (!targetJobParametersBaseDir.exists()) {
        bool created = targetJobParametersBaseDir.mkdir(".");

        if (!created) {
            qCritical() << QString("Could not create archive job parameters dir");
            return false;
        }
    }
    QString targetJobParametersFilePath = targetJobParametersBasePath + "/" + jobFileName;

    bool movedJobFile = QFile::rename(sourceJobFilePath, targetJobFilePath);

    if (movedJobFile) {
        qDebug() << QString("Moved file %1 to %2").arg(sourceJobFilePath).arg(targetJobFilePath);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(sourceJobFilePath).arg(targetJobFilePath);
    }

    bool movedJobParametersFile = QFile::rename(sourceJobParametersFilePath, targetJobParametersFilePath);

    if (movedJobParametersFile) {
        qDebug() << QString("Moved file %1 to %2").arg(sourceJobParametersFilePath).arg(targetJobParametersFilePath);
    } else {
        qCritical() << QString("Could not move file %1 to %2").arg(sourceJobParametersFilePath).arg(targetJobParametersFilePath);
    }

    /* archived jobs collections will be updated when reloading the assembly tree */

    return true;
}

bool ProcessDataManager::duplicateJob(QString jobName, QString newName)
{
    qDebug() << "Duplicating job...";

    if (!_jobs.contains(jobName)) {
        qCritical() << QString("Job '%1' not found in local repository, cannot duplicate").arg(jobName);
        return false;
    }

    JobDefinition *job = _jobs.value(jobName);
    QString newFileName = fromNameToFileName(newName);
    JobDefinition *newJob = job->duplicate(newName, newFileName);
    bool jobFileWritten = writeJobFile(newJob);

    if (!jobFileWritten) {
        qCritical() << QString("Failed to duplicate job '%1'").arg(jobName);
        return false;
    }

    return duplicateElementParameters(job->filename(), newJob->filename(), false);
}

bool ProcessDataManager::duplicateAssembly(QString assemblyName, QString newName)
{
    qDebug() << "Duplicating job...";

    if (!_assemblies.contains(assemblyName)) {
        qCritical() << QString("Assembly '%1' not found in local repository, cannot duplicate").arg(assemblyName);
        return false;
    }

    AssemblyDefinition *assembly = _assemblies.value(assemblyName);
    QString newFileName = fromNameToFileName(newName);
    AssemblyDefinition *newAssembly = assembly->duplicate(newName, newFileName);
    bool assemblyFileWritten = writeAssemblyFile(newAssembly);

    if (!assemblyFileWritten) {
        qCritical() << QString("Failed to duplicate assembly '%1'").arg(assemblyName);
        return false;
    }

    return duplicateElementParameters(assembly->filename(), newFileName, true);
}

QString ProcessDataManager::fromNameToFileName(QString name)
{
    QString fileName = name;
    fileName.replace(" ", "_"); // normalize name
    fileName.append(".xml");    // append file extension
    return fileName;
}

bool ProcessDataManager::assemblyHasArchivedJob(QString assemblyName)
{
    return _archivedJobsByAssembly.contains(assemblyName);
}
