#include "SystemDataManager.h"

using namespace MatisseTools;

SystemDataManager::SystemDataManager() :
    _platformDump(NULL),
    _remotePlatformDump(NULL)
{
}

bool SystemDataManager::readMatisseSettings(QString filename)
{
    QFileInfo fileIn(filename);
    if (!fileIn.exists()) {
        qFatal(QString("Matisse settings file '%1' not found").arg(filename).toLatin1());
    }

    QFile settingsFile(fileIn.absoluteFilePath());

    settingsFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&settingsFile);

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

            if (elementName == "XmlRootDir") {
                _userDataPath = reader.readElementText();
            }

            else if (elementName == "PlatformDir") {
                _platformDumpPath = reader.readElementText();
            }

            else if (elementName == "DllRootDir") {
                _dllPath = reader.readElementText();
            }

            else if (elementName == "Port") {
                bool isOk;
                _port = reader.readElementText().toFloat(&isOk);
                if (!isOk)
                    _port=6666;
            }

            else if (elementName == "Version") {
                _version = reader.readElementText();
            }

            else if (elementName == "DataRootDir") {
                _dataRootDir = reader.readElementText();
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing preferences file :" << reader.error();
        }
    }

    reader.clear();
    settingsFile.close();

    _platformSummaryFilePath = _platformDumpPath + QDir::separator() + "PlatformSummary.xml";
    _platformEnvDumpFilePath = _platformDumpPath + QDir::separator() + "PlatformEnvDump.txt";

    return true;
}

bool SystemDataManager::readMatissePreferences(QString filename, MatissePreferences &prefs)
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

    QXmlStreamReader reader(&prefsFile);

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

            if (elementName == "LastUpdate") {
                QString value = reader.readElementText();
                QDateTime dt = QDateTime::fromString(value, "dd/MM/yyyy hh:mm");
                prefs.setLastUpdate(dt);

            } else if (elementName == "ImportExportPath") {
                prefs.setImportExportPath(reader.readElementText());

            } else if (elementName == "ArchivePath") {
                prefs.setArchivePath(reader.readElementText());

            } else if (elementName == "DefaultResultPath") {
                prefs.setDefaultResultPath(reader.readElementText());

            } else if (elementName == "DefaultMosaicFilenamePrefix") {
                prefs.setDefaultMosaicFilenamePrefix(reader.readElementText());

            } else if (elementName == "ProgrammingModeEnabled") {
                QString value = reader.readElementText();
                bool progEnabled = QVariant(value).toBool();
                prefs.setProgrammingModeEnabled(progEnabled);

            } else if (elementName == "Language") {
                prefs.setLanguage(reader.readElementText());
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing preferences file :" << reader.error();
        }
    }

    reader.clear();
    prefsFile.close();

    return true;
}

bool SystemDataManager::writeMatissePreferences(QString filename, MatissePreferences &prefs)
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

    QXmlStreamWriter writer(&prefsFile);
    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("MatissePreferences");

    writer.writeStartElement("LastUpdate");
    writer.writeCharacters(prefs.lastUpdate().toString("dd/MM/yyyy hh:mm"));
    writer.writeEndElement();

    writer.writeStartElement("ImportExportPath");
    writer.writeCharacters(prefs.importExportPath());
    writer.writeEndElement();

    writer.writeStartElement("ArchivePath");
    writer.writeCharacters(prefs.archivePath());
    writer.writeEndElement();

    writer.writeStartElement("DefaultResultPath");
    writer.writeCharacters(prefs.defaultResultPath());
    writer.writeEndElement();

    writer.writeStartElement("DefaultMosaicFilenamePrefix");
    writer.writeCharacters(prefs.defaultMosaicFilenamePrefix());
    writer.writeEndElement();

    writer.writeStartElement("ProgrammingModeEnabled");
    writer.writeCharacters(QVariant(prefs.programmingModeEnabled()).toString());
    writer.writeEndElement();

    writer.writeStartElement("Language");
    writer.writeCharacters(prefs.language());
    writer.writeEndElement();

    writer.writeEndElement(); // ending start tag

    if (writer.hasError()) {
        qCritical() << "Errors occurred while writing matisse preferences to XML";
        return false;
    }

    prefsFile.close();

    return true;
}

bool SystemDataManager::writePlatformSummary()
{
    if (!_platformDump) {
        getPlatformDump();
    }

    QMap<QString, QString> *componentsInfo = _platformDump->getComponentsInfo();
    QList<QString> componentsNames = componentsInfo->keys();

    QFile platformFile(_platformSummaryFilePath);

    if (platformFile.exists()) {
        qDebug() << "Overwriting platform summary file : " << _platformSummaryFilePath;
    } else {
        qDebug() << "Creating platform summary file : " << _platformSummaryFilePath;
    }

    if (!platformFile.open(QFile::WriteOnly)) {
        qCritical() << "Platform summary file could not be opened.";
        return false;
    }

    QXmlStreamWriter writer(&platformFile);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("MatissePlatformSummary");

    writer.writeStartElement("Matisse");
    QXmlStreamAttribute appVersion("version", _version);
    writer.writeAttribute(appVersion);
    writer.writeEndElement();

    writer.writeStartElement("System");
    QXmlStreamAttribute osName("name", _platformDump->getOsName());
    writer.writeAttribute(osName);
    QXmlStreamAttribute osVersion("version", _platformDump->getOsVersion());
    writer.writeAttribute(osVersion);
    writer.writeEndElement();

    writer.writeStartElement("Dependencies");

    foreach (QString componentName, componentsNames) {
        QString version = componentsInfo->value(componentName);

        writer.writeStartElement("Dependency");
        QXmlStreamAttribute depName("name", componentName);
        writer.writeAttribute(depName);
        QXmlStreamAttribute depVersion("version", version);
        writer.writeAttribute(depVersion);
        writer.writeEndElement();
    }

    writer.writeEndElement();

    writer.writeEndElement();
    writer.writeEndDocument();

    if (writer.hasError()) {
        qCritical() << "Errors occurred while writing matisse preferences to XML";
        return false;
    }

    return true;
}

bool SystemDataManager::readRemotePlatformSummary(QString filename)
{
    QFileInfo fileIn(filename);

    if (!fileIn.exists()) {
        qCritical() << QString("Platform summary file '%1' not found").arg(filename).toLatin1();
        return false;
    }

    if (_remotePlatformDump) {
        // previous remote platform dump is not applicable
        delete _remotePlatformDump;
    }

    // allocate new object to collect info about the imported file originating platform
    _remotePlatformDump = new PlatformDump();

    QFile importedPlatformFile(fileIn.absoluteFilePath());

    importedPlatformFile.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&importedPlatformFile);

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

            if (elementName == "Matisse") {
                QXmlStreamAttributes attr = reader.attributes();
                QString appVersion = attr.value("version").toString();
                _remotePlatformDump->setMatisseVersion(appVersion);
            } else if (elementName == "System") {
                QXmlStreamAttributes attr = reader.attributes();
                QString systemName = attr.value("name").toString();
                QString systemVersion = attr.value("version").toString();
                _remotePlatformDump->setOsName(systemName);
                _remotePlatformDump->setOsVersion(systemVersion);
            } else if (elementName == "Dependency") {
                QXmlStreamAttributes attr = reader.attributes();
                QString depName = attr.value("name").toString();
                QString depVersion = attr.value("version").toString();
                _remotePlatformDump->addComponentInfo(depName, depVersion);
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing platform summary file :" << reader.error();
        }
    }

    reader.clear();
    importedPlatformFile.close();

    return true;
}


bool SystemDataManager::writePlatformEnvDump()
{
    if (!_platformDump) {
        getPlatformDump();
    }

    QString envDump = _platformDump->getEnvVariables();

    if (envDump.isEmpty()) {
        qCritical() << "Could not retrieve environment variables. No dump file will be created";
        return false;
    }

    QFile envDumpFile(_platformEnvDumpFilePath);

    if (envDumpFile.exists()) {
        qDebug() << "Overwriting platform environment dump file : " << _platformEnvDumpFilePath;
    } else {
        qDebug() << "Creating platform environment dump file : " << _platformEnvDumpFilePath;
    }

    if (!envDumpFile.open(QFile::WriteOnly)) {
        qCritical() << "Platform environment dump file could not be opened.";
        return false;
    }

    QTextStream writer(&envDumpFile);
    writer << envDump;
    writer.flush();
    envDumpFile.close();

    return true;
}

PlatformComparisonStatus *SystemDataManager::compareRemoteAndLocalPlatform()
{
    if (!_platformDump) {
        getPlatformDump();
    }

    if (!_remotePlatformDump) {
        qCritical() << "Remote platform dump not found. Could not compare remote and local platform";
        return NULL;
    }

    PlatformComparisonStatus *status = _platformComparator.compare(_platformDump, _remotePlatformDump);

    return status;
}

void SystemDataManager::getPlatformDump()
{
    qDebug() << "Retrieving local platform dump...";
    _platformInspector.init();
    _platformInspector.inspect();
    _platformDump = _platformInspector.getDump();
    _platformDump->setMatisseVersion(_version);
}

QString SystemDataManager::getDataRootDir() const
{
    return _dataRootDir;
}

QString SystemDataManager::getPlatformEnvDumpFilePath() const
{
    return _platformEnvDumpFilePath;
}

QString SystemDataManager::getPlatformSummaryFilePath() const
{
    return _platformSummaryFilePath;
}



