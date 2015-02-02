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
    qDebug()<< "readJobFile "  << filename;


    QFileInfo fileInfo(_jobsPath + QDir::separator() + filename);


//    if (!xmlIsValid(_assembliesSchema, fileInfo)) {
//        return false;
//    }

    qDebug()<< "Loading job " << fileInfo.absoluteFilePath();

    QFile jobFile(fileInfo.absoluteFilePath());
    jobFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&jobFile);

    JobDefinition *newJob = NULL;
    ExecutionDefinition *executionDefinition = NULL;

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
    }
    /* Removes any device() or data from the reader
         * and resets its internal state to the initial state. */
    reader.clear();

    if (newJob) {
        _jobs.insert(newJob->name(), newJob);
        return true;
    }
    return false;

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
        qDebug() << "Invalid query:" << queryRootDir;
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
    _jobsPath = _basePath + QDir::separator() + "users" + QDir::separator() + "jobs";
    _jobsParametersPath =  _basePath + QDir::separator() + "users" + QDir::separator() + "parameters";
    _assembliesPath = _basePath + QDir::separator() + "assemblies";
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

QString Xml::getJobsParametersPath(QString parameterVersion, QString parameterName)
{
    return QDir::cleanPath( _jobsParametersPath
                            + QDir::separator()
                            + parameterVersion
                            + QDir::separator()
                            + parameterName.replace(" ", "_") + ".xml"
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
        qDebug() << "Error finding" << assembliesXsd.fileName();
        return false;
    }

    if (!assembliesXsd.open(QIODevice::ReadOnly)) {
        qDebug() << "Error opening MatisseAssemblies.xsd";
        return false;
    }

    if (!_assembliesSchema.load(&assembliesXsd, QUrl::fromLocalFile(assembliesXsd.fileName()))) {
        qDebug() << "Error loading MatisseAssemblies.xsd";
        return false;
    }

    if (!_assembliesSchema.isValid()) {
        qDebug() << "Error MatisseAssemblies.xsd is not valid";
        return false;
    }

    assembliesXsd.close();

    return true;

}

bool Xml::xmlIsValid(QXmlSchema &schema, QFileInfo fileInfo)
{

    if (!schema.isValid()) {
        qDebug() << "Error validation schema invalid";
        return false;
    }
    ;

    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error opening file" << QFileInfo(file).absoluteFilePath() ;
        return false;
    }

    QXmlSchemaValidator validator(schema);
    try {
        if (!validator.validate(&file, QUrl::fromLocalFile(file.fileName()))) {
            qDebug() << "Error:" <<  QFileInfo(file).absoluteFilePath()  << "is not compliant with the schema";
            file.close();
            return false;
        }
    } catch (std::exception &excep) {
        qDebug() << "Erreur fichier" << file.fileName();
        qDebug() << "Exception:" << excep.what();
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
