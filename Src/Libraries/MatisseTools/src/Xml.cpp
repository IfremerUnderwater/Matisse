#include "Xml.h"

using namespace MatisseTools;

Xml::Xml()
{
}



///
/// \brief Xml::readAssemblyFile
/// \param filename chemin relatif aux assemblages
/// \return
///
bool Xml::readAssemblyFile(QString filename)
{
    qDebug()<< "readAssemblyFile "  << filename;

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


    qDebug()<< "Recherche de l'assemblage " << fileInfo.absoluteFilePath();

    QFile assemblyFile(fileInfo.absoluteFilePath());
    assemblyFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&assemblyFile);

    AssemblyDefinition *newAssembly = NULL;

    bool startDescriptor = false;
    bool endDescriptor = false;

    while(!reader.atEnd() &&
               !reader.hasError()) {
        /* Read next element.*/
        QXmlStreamReader::TokenType token = reader.readNext();
        /* If token is just StartDocument, we'll go to next.*/
        if(token == QXmlStreamReader::StartDocument) {
           continue;
        }
        /* If token is StartElement, we'll see if we can read it.*/
        if(token == QXmlStreamReader::StartElement) {
            QString elementName = reader.name().toString();
            //qDebug() << "Find element " << elementName;

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
                if (newAssembly) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString model = attributes.value("model").toString();
                    QString name = attributes.value("name").toString();
                    ParameterDefinition* parameterDefinition = new ParameterDefinition(model, name);
                    newAssembly->setParametersDefinition(parameterDefinition);
                }
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
    }
    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();

    if (newAssembly) {
        _assemblies.insert(newAssembly->name(), newAssembly);
        qDebug() << "ASSEMBLY DEFINITION VALIDITY" << newAssembly->checkDefinition();
        return true;
    }
    return false;

}

///
/// \brief Xml::readJobFile
/// \param filename chemin relatif aux assemblages
/// \return
///
bool Xml::readJobFile(QString filename)
{
    QString jobFilePath = _jobsPath + QDir::separator() + filename;

    qDebug()<< "Reading job file "  << jobFilePath;


    QFileInfo fileInfo(jobFilePath);


//    if (!xmlIsValid(_assembliesSchema, fileInfo)) {
//        return false;
//    }

    qDebug()<< "Loading job " << fileInfo.absoluteFilePath();

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
            else if ("Parameters" == elementName) {
                if (newJob) {
                    QXmlStreamAttributes attributes = reader.attributes();
                    QString model = attributes.value("model").toString();
                    QString name = attributes.value("name").toString();
                    ParameterDefinition* parameterDefinition = new ParameterDefinition(model, name);
                    newJob->setParametersDefinition(parameterDefinition);
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

    _jobs.insert(newJob->name(), newJob);
    return true;
}

bool Xml::updateJobFile(QString jobName)
{
    return writeJobFile(jobName, true);
}

bool Xml::writeJobFile(QString jobName, bool overWrite)
{
    JobDefinition * job = getJob(jobName);
    if (!job) {
        return false;
    }
    QFile jobFile(getJobsPath() + QDir::separator() + jobName + ".xml");

    if (jobFile.exists() && (!overWrite)) {
        return false;
    }

    if (!jobFile.open(QFile::WriteOnly)) {
        return false;
    }

    QTextStream os(&jobFile);
    os << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    os << job->serialized();

    os.flush();

    jobFile.close();

    return true;
}

// TODO Ne marche pas encore
// La structure AssemblyDefinition en cours de création n'est pas complète
// La structure à jour est défini par les collections de widgets dans
// AssemblyGraphicsScene
bool Xml::saveAssembly(QString filename, AssemblyDefinition *assembly)
{
    if (!assembly) {
        qWarning() << "Assembly is null : could not be saved";
        return false;
    }

    // TODO: vérification, de la présence des paramètres
    // TODO: vérification de la liste des entrées
    QFile assemblyFile(filename);
    QTextStream os(&assemblyFile);
    os.setCodec("UTF-8"); // forcer l'encodage en UTF-8, sinon les caractères accentués
                          // mal encodés empêchent de relire l'assemblage
    if (!assemblyFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Erreur ouverture ecriture";
        return false;
    }

    bool valid = false;

    QString name = assembly->name();

    os << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    os << QString("<MatisseAssembly name=\"%1\" isRealTime=\"%2\" usable=\"%3\">\n").arg(name).arg(assembly->isRealTime()).arg(valid);
    os << "\t<DescriptorFields>\n";

    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Author").arg(assembly->author());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Version").arg(assembly->version());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Comments").arg(assembly->comment());
    os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg("Date").arg(assembly->date());

//    QStringList fieldsKeys = QStringList() << "Author" << "Version" << "Comments" << "Date";
//    foreach(QString key, fieldsKeys) {
//        // test manuel pour l'instant...
//        os << QString("\t\t<%1>\n\t\t\t%2\n\t\t</%1>\n").arg(key).arg(fields.getValue(key));
//    }
    os << "\t</DescriptorFields>\n";
    // On teste tout au cas ou l'assemblage ne serait pas valide...
    // ecriture des parameters
//    QString parametersName = "";
//    QString parametersVersion = "";
//    if (_parametersWidget) {
//        QStringList args = _parametersWidget ->getName().split("\n");
//        if (args.size() > 1) {
//            parametersVersion = args[0];
//            parametersName = args[1];
//        }
//    }
    os << QString("\t<Parameters id=\"99\" model=\"V1.0\" name=\"VictorOTUSCongo\"/>\n");
    os << "\n";
    // ecriture de la source
    QString sourceName = assembly->sourceDefinition()->name();
//    if (_sourceWidget) {
//        sourceName = _sourceWidget->getName();
//    }

    os << QString("\t<Source id=\"99\" name=\"%1\" order=\"0\"/>\n").arg(sourceName);
    os << "\n";
    // ecriture des processeurs
    os << "\t<Processors>\n";
//    foreach(quint8 procPos, _processorsWidgets.keys()) {
//        ProcessorWidget * curProc = _processorsWidgets.value(procPos);
//        os << QString("\t\t<Processor id=\"%1\" name=\"%2\" order=\"%3\"/>\n").arg(99).arg(curProc->getName()).arg(procPos);
//    }

    foreach(ProcessorDefinition* processorDef, assembly->processorDefs()) {
        os << QString("\t\t<Processor id=\"99\" name=\"%1\" order=\"%2\"/>\n").arg(processorDef->name()).arg(processorDef->order());
    }

    os << "\t</Processors>\n";
    os << "\n";

    // ecriture de la destination
    QString destinationName = assembly->destinationDefinition()->name();
    QString destinationOrder = assembly->destinationDefinition()->order();
//    if (_destinationWidget) {
//        destinationName = QString("%1").arg(_destinationWidget->getName());
//        destinationOrder = QString("%1").arg(_destinationWidget->getOrder());
//    }
    os << QString("\t<Destination id=\"99\" name=\"%1\" order=\"%2\"/>\n").arg(99).arg(destinationName).arg(destinationOrder);
    os << "\n";
    // ecriture des relations
    os << "\t<Connections>\n";
//    foreach(PipeWidget * pipe, _connectors) {
//        qint8 order1 = pipe->getStartElement()->getOrder();
//        qint8 line1 = pipe->getStartElementLine();
//        qint8 order2 = pipe->getEndElement()->getOrder();
//        qint8 line2 = pipe->getEndElementLine();
//        QColor color = pipe->getColor();
//        os << QString("\t\t<Connection startOrder=\"%1\" startLine=\"%2\" endOrder=\"%3\" endLine=\"%4\" color=\"%5\"/>\n").arg(order1).arg(line1).arg(order2).arg(line2).arg(color.rgba());
//    }

    foreach(ConnectionDefinition *connectionDef, assembly->connectionDefs()) {
        qint32 order1 = connectionDef->startOrder();
        qint32 line1 = connectionDef->startLine();
        qint32 order2 = connectionDef->endOrder();
        qint32 line2 = connectionDef->endLine();
        QRgb color = connectionDef->color();

        //QColor color = pipe->getColor();
        os << QString("\t\t<Connection startOrder=\"%1\" startLine=\"%2\" endOrder=\"%3\" endLine=\"%4\" color=\"%5\"/>\n").arg(order1).arg(line1).arg(order2).arg(line2).arg(color);
    }

    os << "\t</Connections>\n";
    os << "</MatisseAssembly>";
    os.flush();
    assemblyFile.close();

    return true;
}

bool Xml::writeJobFile(JobDefinition * job, bool overWrite)
{
    if (!job) {
        return false;
    }
    QString filename = job->filename();
    if (filename.isEmpty()) {
        filename = job->name();
        filename.replace(" ", "_").append(".xml");
        job->setFilename(filename);
    }
    QFile jobFile(getJobsPath() + QDir::separator() + filename);

    if (jobFile.exists() && (!overWrite)) {
        return false;
    }

    if (!jobFile.open(QFile::WriteOnly)) {
        return false;
    }

    QTextStream os(&jobFile);
    os << QString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    os << job->serialized();

    os.flush();

    jobFile.close();

    return true;
}

bool Xml::readMatisseGuiSettings(QString filename)
{
    // TODO: modifier signature: retour bool + liste key/values....
    QFileInfo fileIn(filename);

    QXmlQuery query;
    QString queryRootDir = "doc('" + fileIn.absoluteFilePath() + "')//MatisseSettings";

    query.setQuery(queryRootDir);

    if(!query.isValid()) {
        qWarning() << "Invalid query:" << queryRootDir;
        return false;
    }

    QString bufferStr;

    query.evaluateTo(&bufferStr);
    QStringList settingsFields = bufferStr.split("\n");

    foreach(QString settingsField, settingsFields) {
        QXmlStreamReader reader(settingsField);
        if (reader.readNextStartElement()) {
            if (reader.name() == "XmlRootDir") {
                _basePath = reader.readElementText();
                //qDebug()<< "BasePath : " << _basePath;
            }
            else if (reader.name() == "DllRootDir") {
                _dllPath = reader.readElementText();
                //qDebug()<< "DllPath : " << _dllPath;
            }
            else if (reader.name() == "Port") {
                bool isOk;
                _port = reader.readElementText().toFloat(&isOk);
                if (!isOk)
                    _port=6666;
                //qDebug()<< "Port : " << _port;
            }
        }
    }
    _jobsPath = _basePath + QDir::separator() + "jobs";
    _jobsParametersPath =  _basePath + QDir::separator() + "jobs" + QDir::separator() + "parameters";
    _assembliesPath = _basePath + QDir::separator() + "assemblies";
    _assembliesParametersPath = _assembliesPath + QDir::separator() + "parameters";
    return true;
}

bool Xml::readMatissePreferences(QString filename, MatissePreferences &prefs)
{
    QFile prefsFile(filename);

    if (!prefsFile.exists()) {
        qCritical() << "Preferences file does not exist : " << filename;
        return false;
    }

    if (!prefsFile.open(QFile::ReadOnly)) {
        qCritical() << "Preferences file could not be opened.";
        return false;
    }

    qDebug() << "Reading preferences file...";

    QXmlStreamReader xsr(&prefsFile);

    while (xsr.readNextStartElement()) {
        if (xsr.name() == "LastUpdate") {
            QString value = xsr.readElementText();
            QDateTime dt = QDateTime::fromString(value, "dd/MM/yyyy hh:mm");
            prefs.setLastUpdate(dt);

        } else if (xsr.name() == "ImportExportPath") {
            prefs.setImportExportPath(xsr.readElementText());

        } else if (xsr.name() == "ArchivePath") {
            prefs.setArchivePath(xsr.readElementText());

        } else if (xsr.name() == "DefaultResultPath") {
            prefs.setDefaultResultPath(xsr.readElementText());

        } else if (xsr.name() == "DefaultMosaicFilenamePrefix") {
            prefs.setDefaultMosaicFilenamePrefix(xsr.readElementText());

        } else if (xsr.name() == "ProgrammingModeEnabled") {
            QString value = xsr.readElementText();
            bool progEnabled = QVariant(value).toBool();
            prefs.setProgrammingModeEnabled(progEnabled);

        } else if (xsr.name() == "Language") {
            prefs.setLanguage(xsr.readElementText());
        }
    }

    if (xsr.hasError()) {
        xsr.raiseError();
        qCritical() << "Error while parsing preferences file :" << xsr.error();
    }

    return true;
}

bool Xml::writeMatissePreferences(QString filename, MatissePreferences &prefs)
{
    QFile prefsFile(filename);

    if (prefsFile.exists()) {
        qDebug() << "Overwriting preferences file : " << filename;
    } else {
        qDebug() << "Creating preferences file : " << filename;
    }

    if (!prefsFile.open(QFile::WriteOnly)) {
        qCritical() << "Preferences file could not be opened.";

        return false;
    }

    QXmlStreamWriter xsw(&prefsFile);
    xsw.setCodec("UTF-8");
    xsw.setAutoFormatting(true);
    xsw.writeStartDocument();
    xsw.writeStartElement("MatissePreferences");

    xsw.writeStartElement("LastUpdate");
    xsw.writeCharacters(prefs.lastUpdate().toString("dd/MM/yyyy hh:mm"));
    xsw.writeEndElement();

    xsw.writeStartElement("ImportExportPath");
    xsw.writeCharacters(prefs.importExportPath());
    xsw.writeEndElement();

    xsw.writeStartElement("ArchivePath");
    xsw.writeCharacters(prefs.archivePath());
    xsw.writeEndElement();

    xsw.writeStartElement("DefaultResultPath");
    xsw.writeCharacters(prefs.defaultResultPath());
    xsw.writeEndElement();

    xsw.writeStartElement("DefaultMosaicFilenamePrefix");
    xsw.writeCharacters(prefs.defaultMosaicFilenamePrefix());
    xsw.writeEndElement();

    xsw.writeStartElement("ProgrammingModeEnabled");
    xsw.writeCharacters(QVariant(prefs.programmingModeEnabled()).toString());
    xsw.writeEndElement();

    xsw.writeStartElement("Language");
    xsw.writeCharacters(prefs.language());
    xsw.writeEndElement();

    xsw.writeEndElement(); // ending start tag

    if (xsw.hasError()) {
        qCritical() << "Errors occurred while writing matisse preferences to XML";
        return false;
    }

    prefsFile.close();

    return true;
}

bool Xml::validateXmlFile(QString xmlSchema, QString xmlFile)
{
    bool ret = false;
    QXmlSchema schema;
        schema.load(QUrl::fromLocalFile(xmlSchema));
        if ( schema.isValid() ) {
            if (!xmlFile.isEmpty()) {
                QXmlSchemaValidator validator( schema );
                if ( validator.validate(QUrl::fromLocalFile(xmlFile))) {
                    qDebug() << "instance is valid";
                    ret = true;
                } else {
                    qDebug() << "instance is invalid";
                }
            } else {
                ret = true;
            }
        } else {
            qDebug() << "schema is invalid";
        }

        return ret;
}

QStringList Xml::getAssembliesList()
{
    return _assemblies.keys();
}

QString Xml::getAssembliesPath()
{
    return _assembliesPath;
}

AssemblyDefinition *Xml::getAssembly(QString assemblyName)
{
    return _assemblies.value(assemblyName, NULL);
}

JobDefinition *Xml::getJob(QString jobName)
{
    return _jobs.value(jobName, NULL);
}

QStringList Xml::getJobsNames()
{
    return _jobs.keys();
}

QString Xml::getModelPath(QString parameterVersion)
{
    return  QDir::cleanPath( _basePath
                             + QDir::separator()
                             + "models"
                             + QDir::separator()
                             + "parameters"
                             + QDir::separator()
                             + "Parameters_"
                             + parameterVersion
                             + ".xml"
                             );
}

QString Xml::getJobsPath()
{
    return _jobsPath;
}

QString Xml::getJobsParametersPath(QString jobName)
{
    return QDir::cleanPath( _jobsParametersPath
                            + QDir::separator()
                            + jobName + ".xml"
                            );
}

QString Xml::getAssembliesParametersPath(QString parameterVersion, QString parameterName)
{
    return QDir::cleanPath(_basePath
                           + QDir::separator()
                           + "parameters"
                           + QDir::separator()
                           + parameterVersion
                           + QDir::separator()
                           + parameterName.replace(" ", "_") + ".xml"
                           );
}

bool Xml::loadModels()
{

    QFile assembliesXsd(_basePath + "/models/MatisseAssembly.xsd");

    if (!assembliesXsd.exists()) {
        qCritical() << "Error finding" << assembliesXsd.fileName();
        return false;
    }

    if (!assembliesXsd.open(QIODevice::ReadOnly)) {
        qCritical() << "Error opening MatisseAssemblies.xsd";
        return false;
    }

    if (!_assembliesSchema.load(&assembliesXsd, QUrl::fromLocalFile(assembliesXsd.fileName()))) {
        qCritical() << "Error loading MatisseAssemblies.xsd";
        return false;
    }

    if (!_assembliesSchema.isValid()) {
        qCritical() << "Error MatisseAssemblies.xsd is not valid";
        return false;
    }

    assembliesXsd.close();

    return true;

}

bool Xml::xmlIsValid(QXmlSchema &schema, QFileInfo fileInfo)
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

// lecture des descripteurs
// on suppose que le fichier est valide...

KeyValueList Xml::readParametersFileDescriptor(QString filename)
{
    KeyValueList keyValueList;
    QFileInfo fileIn(filename);

    QXmlQuery query;
    QString queryDescriptor = "doc('" + fileIn.absoluteFilePath() + "')//fileDescriptor/*";

    query.setQuery(queryDescriptor);

    if(!query.isValid()) {
        qDebug() << "Invalid query:" << queryDescriptor;
        return keyValueList;
    }

    QString bufferStr;

    query.evaluateTo(&bufferStr);
    QStringList fields = bufferStr.split("\n");
    foreach(QString field, fields) {
    QXmlStreamReader reader(field);

    if (reader.readNextStartElement()) {
        QString name(reader.name().toUtf8());
        if (name != "") {
            reader.readNext();
            QString value(reader.text().toUtf8());
            keyValueList.append(name, value);
        }
    }
}

    return keyValueList;
}

void Xml::clearAssembliesDatas()
{
    // TODO: remplacer les pointeur par des shared pointer et enlever les references dans le clear...
    _assemblies.clear();
}

void Xml::clearJobsDatas()
{
    // TODO: remplacer les pointeur par des shared pointer et enlever les references dans le clear...
    _jobs.clear();

}
