#include "system_data_manager.h"

namespace matisse {

//// Singleton constructor
SystemDataManager* SystemDataManager::instance()
{
    // create instance by lazy initialization
    // guaranteed to be destroyed
    // The instance object is created once and is persistent
    // between 2 invocations of the instance() method
    static SystemDataManager instance;

    return &instance;
}

SystemDataManager::SystemDataManager() :
  m_platform_dump(NULL),
  m_remote_platform_dump(NULL)
{
    m_remote_server_settings = new MatisseRemoteServerSettings();
}

SystemDataManager::~SystemDataManager()
{

}

void SystemDataManager::init(QString _bin_root_dir)
{
    m_bin_root_dir = _bin_root_dir;
}

bool SystemDataManager::readMatisseSettings(QString _filename)
{
    if (m_bin_root_dir.isEmpty()) {
        qFatal("System data manager not initialized");
    }

    QFileInfo file_in(_filename);
    if (!file_in.exists()) {
        qFatal("%s\n",QString("Matisse settings file '%1' not found").arg(_filename).toStdString().c_str());
    }

    QFile settings_file(file_in.absoluteFilePath());

    settings_file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&settings_file);

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

            if (element_name == "XmlRootDir") {
                m_user_data_path = reader.readElementText();
            }

            else if (element_name == "PlatformDir") {
                m_platform_dump_path = reader.readElementText();
            }

            else if (element_name == "DllRootDir") {
                m_dll_path = reader.readElementText();
            }

            else if (element_name == "Port") {
                bool is_ok;
                m_port = reader.readElementText().toFloat(&is_ok);
                if (!is_ok)
                    m_port=6666;
            }

            else if (element_name == "Version") {
                m_version = reader.readElementText();
            }

            else if (element_name == "DataRootDir") {
                m_data_root_dir = reader.readElementText();
            }

            /* Remote server settings */
            else if (element_name == "RemoteServerSetting") {
              QXmlStreamAttributes attributes = reader.attributes();
              QString server_setting_name = attributes.value("name").toString();
              QString path = attributes.value("path").toString();
              QString alias = attributes.value("alias").toString();
              
              RemoteServerSetting server_setting;
              server_setting.setPath(path);
              server_setting.setAlias(alias);

              if (server_setting_name == "containerImage") {
                m_remote_server_settings->setContainerImage(server_setting);
              }              
              else if (server_setting_name == "launcherParentDir") {
                m_remote_server_settings->setLauncherParentDir(server_setting);
              }              
              else if (server_setting_name == "launcherFile") {
                m_remote_server_settings->setLauncherFile(server_setting);
              }              
              else if (server_setting_name == "binRoot") {
                m_remote_server_settings->setBinRoot(server_setting);
              }              
              else if (server_setting_name == "datasetsDir") {
                m_remote_server_settings->setDatasetsDir(server_setting);
              }
              else if (server_setting_name == "applicationFilesRoot") {
                m_remote_server_settings->setApplicationFilesRoot(server_setting);
              }              
              else if (server_setting_name == "jobsSubdir") {
                m_remote_server_settings->setJobsSubdir(server_setting);
              }              
              else if (server_setting_name == "resultsSubdir") {
                m_remote_server_settings->setResultsSubdir(server_setting);
              } else {
                qWarning() << "SystemDataManager.readMatisseSettings: unknown "
                              "remote server setting : "
                           << server_setting_name;
              }              
            }

            /* External tools definition */
            else if (element_name == "ExternalTool") {
                QXmlStreamAttributes attributes = reader.attributes();
                QString tool_name = attributes.value("name").toString();
                QString tool_exe_path = attributes.value("exePath").toString();

                if (m_external_tools.contains(tool_name)) {
                    qCritical() << QString("External tool '%1' is defined more than once").arg(tool_name);
                    continue;
                }

                m_external_tools.insert(tool_name, tool_exe_path);
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing preferences file :" << reader.error();
        }
    }

    reader.clear();
    settings_file.close();

    // TODO : use AppDataLocation once all paths have been transferred (including job files)
//    m_platform_dump_path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
//            QDir::separator() + "platform";


    m_platform_summary_file_path = m_platform_dump_path + QDir::separator() + "PlatformSummary.xml";
    m_platform_env_dump_file_path = m_platform_dump_path + QDir::separator() + "PlatformEnvDump.txt";

    return true;
}

bool SystemDataManager::readMatissePreferences(QString _filename, MatissePreferences &_prefs)
{
    QFile prefs_file(_filename);

    if (!prefs_file.exists()) {
        qCritical() << "Preferences file does not exist : " << _filename;
        return false;
    }

    if (!prefs_file.open(QFile::ReadOnly)) {
        qCritical() << "Preferences file could not be opened.";
        return false;
    }

    qDebug() << "Reading preferences file...";

    QXmlStreamReader reader(&prefs_file);

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

            if (element_name == "LastUpdate") {
                QString value = reader.readElementText();
                QDateTime dt = QDateTime::fromString(value, "dd/MM/yyyy hh:mm");
                _prefs.setLastUpdate(dt);

            } else if (element_name == "ImportExportPath") {
                _prefs.setImportExportPath(reader.readElementText());

            } else if (element_name == "ArchivePath") {
                _prefs.setArchivePath(reader.readElementText());

            } else if (element_name == "DefaultResultPath") {
                _prefs.setDefaultResultPath(reader.readElementText());

            } else if (element_name == "DefaultMosaicFilenamePrefix") {
                _prefs.setDefaultMosaicFilenamePrefix(reader.readElementText());

            } else if (element_name == "ProgrammingModeEnabled") {
                QString value = reader.readElementText();
                bool prog_enabled = QVariant(value).toBool();
                _prefs.setProgrammingModeEnabled(prog_enabled);

            } else if (element_name == "Language") {
                _prefs.setLanguage(reader.readElementText());
            }
            else if (element_name == "RemoteCommandServer") {
              _prefs.setRemoteCommandServer(reader.readElementText());
            }
            else if (element_name == "RemoteFileServer") {
              _prefs.setRemoteFileServer(reader.readElementText());
            }
            else if (element_name == "RemoteUsername") {
              _prefs.setRemoteUsername(reader.readElementText());
            }
            else if (element_name == "RemoteUserEmail") {
              _prefs.setRemoteUserEmail(reader.readElementText());
            }
            else if (element_name == "RemoteQueueName") {
              _prefs.setRemoteQueueName(reader.readElementText());
            }
            else if (element_name == "RemoteNbOfCpus") {
              QString value = reader.readElementText();
              int nb_of_cpus = QVariant(value).toInt();
              _prefs.setRemoteNbOfCpus(nb_of_cpus);
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing preferences file :" << reader.error();
        }
    }

    reader.clear();
    prefs_file.close();

    return true;
}

bool SystemDataManager::writeMatissePreferences(QString _filename, MatissePreferences &_prefs)
{
    QFile prefs_file(_filename);

    if (prefs_file.exists()) {
        qDebug() << "Overwriting preferences file : " << _filename;
    } else {
        qDebug() << "Creating preferences file : " << _filename;
    }

    if (!prefs_file.open(QFile::WriteOnly)) {
        qCritical() << "Preferences file could not be opened.";

        return false;
    }

    QXmlStreamWriter writer(&prefs_file);
    writer.setCodec("UTF-8");
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("MatissePreferences");

    writer.writeStartElement("LastUpdate");
    writer.writeCharacters(_prefs.lastUpdate().toString("dd/MM/yyyy hh:mm"));
    writer.writeEndElement();

    writer.writeStartElement("ImportExportPath");
    writer.writeCharacters(_prefs.importExportPath());
    writer.writeEndElement();

    writer.writeStartElement("ArchivePath");
    writer.writeCharacters(_prefs.archivePath());
    writer.writeEndElement();

    writer.writeStartElement("DefaultResultPath");
    writer.writeCharacters(_prefs.defaultResultPath());
    writer.writeEndElement();

    writer.writeStartElement("DefaultMosaicFilenamePrefix");
    writer.writeCharacters(_prefs.defaultMosaicFilenamePrefix());
    writer.writeEndElement();

    writer.writeStartElement("ProgrammingModeEnabled");
    writer.writeCharacters(QVariant(_prefs.programmingModeEnabled()).toString());
    writer.writeEndElement();

    writer.writeStartElement("Language");
    writer.writeCharacters(_prefs.language());
    writer.writeEndElement();

    writer.writeStartElement("RemoteCommandServer");
    writer.writeCharacters(_prefs.remoteCommandServer());
    writer.writeEndElement();
    
    writer.writeStartElement("RemoteFileServer");
    writer.writeCharacters(_prefs.remoteFileServer());
    writer.writeEndElement();

    writer.writeStartElement("RemoteUsername");
    writer.writeCharacters(_prefs.remoteUsername());
    writer.writeEndElement();

    writer.writeStartElement("RemoteUserEmail");
    writer.writeCharacters(_prefs.remoteUserEmail());
    writer.writeEndElement();

    writer.writeStartElement("RemoteQueueName");
    writer.writeCharacters(_prefs.remoteQueueName());
    writer.writeEndElement();

    writer.writeStartElement("RemoteNbOfCpus");
    writer.writeCharacters(QVariant(_prefs.remoteNbOfCpus()).toString());
    writer.writeEndElement();

    writer.writeEndElement(); // ending start tag

    if (writer.hasError()) {
        qCritical() << "Errors occurred while writing matisse preferences to XML";
        return false;
    }

    prefs_file.close();

    return true;
}

bool SystemDataManager::writePlatformSummary()
{
    if (!m_platform_dump) {
        getPlatformDump();
    }

    QMap<QString, QString> *components_info = m_platform_dump->getComponentsInfo();
    QList<QString> components_names = components_info->keys();

    // create platform dir if it does not exist
    QDir platform_dump_dir(m_platform_dump_path);
    if (!platform_dump_dir.exists()) {
        platform_dump_dir.mkpath(".");
    }

    QFile platform_file(m_platform_summary_file_path);

    if (platform_file.exists()) {
        qDebug() << "Overwriting platform summary file : " << m_platform_summary_file_path;
    } else {
        qDebug() << "Creating platform summary file : " << m_platform_summary_file_path;
    }

    if (!platform_file.open(QFile::WriteOnly)) {
        qCritical() << "Platform summary file could not be opened.";
        return false;
    }

    QXmlStreamWriter writer(&platform_file);

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("MatissePlatformSummary");

    writer.writeStartElement("Matisse");
    QXmlStreamAttribute app_version("version", m_version);
    writer.writeAttribute(app_version);
    writer.writeEndElement();

    writer.writeStartElement("System");
    QXmlStreamAttribute os_name("name", m_platform_dump->getOsName());
    writer.writeAttribute(os_name);
    QXmlStreamAttribute os_version("version", m_platform_dump->getOsVersion());
    writer.writeAttribute(os_version);
    writer.writeEndElement();

    writer.writeStartElement("Dependencies");

    foreach (QString component_name, components_names) {
        QString version = components_info->value(component_name);

        writer.writeStartElement("Dependency");
        QXmlStreamAttribute dep_name("name", component_name);
        writer.writeAttribute(dep_name);
        QXmlStreamAttribute dep_version("version", version);
        writer.writeAttribute(dep_version);
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

bool SystemDataManager::readRemotePlatformSummary(QString _filename)
{
    QFileInfo file_in(_filename);

    if (!file_in.exists()) {
        qCritical() << QString("Platform summary file '%1' not found").arg(_filename).toLatin1();
        return false;
    }

    if (m_remote_platform_dump) {
        // previous remote platform dump is not applicable
        delete m_remote_platform_dump;
    }

    // allocate new object to collect info about the imported file originating platform
    m_remote_platform_dump = new PlatformDump();

    QFile imported_platform_file(file_in.absoluteFilePath());

    imported_platform_file.open(QIODevice::ReadOnly);

    QXmlStreamReader reader(&imported_platform_file);

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

            if (element_name == "Matisse") {
                QXmlStreamAttributes attr = reader.attributes();
                QString app_version = attr.value("version").toString();
                m_remote_platform_dump->setMatisseVersion(app_version);
            } else if (element_name == "System") {
                QXmlStreamAttributes attr = reader.attributes();
                QString system_name = attr.value("name").toString();
                QString system_version = attr.value("version").toString();
                m_remote_platform_dump->setOsName(system_name);
                m_remote_platform_dump->setOsVersion(system_version);
            } else if (element_name == "Dependency") {
                QXmlStreamAttributes attr = reader.attributes();
                QString dep_name = attr.value("name").toString();
                QString dep_version = attr.value("version").toString();
                m_remote_platform_dump->addComponentInfo(dep_name, dep_version);
            }
        }

        if (reader.hasError()) {
            qWarning() << "Error while parsing platform summary file :" << reader.error();
        }
    }

    reader.clear();
    imported_platform_file.close();

    return true;
}


bool SystemDataManager::writePlatformEnvDump()
{
    if (!m_platform_dump) {
        getPlatformDump();
    }

    QString env_dump = m_platform_dump->getEnvVariables();

    if (env_dump.isEmpty()) {
        qCritical() << "Could not retrieve environment variables. No dump file will be created";
        return false;
    }

    QFile env_dump_file(m_platform_env_dump_file_path);

    if (env_dump_file.exists()) {
        qDebug() << "Overwriting platform environment dump file : " << m_platform_env_dump_file_path;
    } else {
        qDebug() << "Creating platform environment dump file : " << m_platform_env_dump_file_path;
    }

    if (!env_dump_file.open(QFile::WriteOnly)) {
        qCritical() << "Platform environment dump file could not be opened.";
        return false;
    }

    QTextStream writer(&env_dump_file);
    writer << env_dump;
    writer.flush();
    env_dump_file.close();

    return true;
}

PlatformComparisonStatus *SystemDataManager::compareRemoteAndLocalPlatform()
{
    if (!m_platform_dump) {
        getPlatformDump();
    }

    if (!m_remote_platform_dump) {
        qCritical() << "Remote platform dump not found. Could not compare remote and local platform";
        return NULL;
    }

    PlatformComparisonStatus *status = m_platform_comparator.compare(m_platform_dump, m_remote_platform_dump);

    return status;
}

void SystemDataManager::getPlatformDump()
{
    qDebug() << "Retrieving local platform dump...";
    m_platform_inspector.init();
    m_platform_inspector.inspect();
    m_platform_dump = m_platform_inspector.getDump();
    m_platform_dump->setMatisseVersion(m_version);
}
QMap<QString, QString> SystemDataManager::getExternalTools() const
{
    return m_external_tools;
}


QString SystemDataManager::getDataRootDir() const
{
    return m_data_root_dir;
}

QString SystemDataManager::getPlatformEnvDumpFilePath() const
{
    return m_platform_env_dump_file_path;
}

QString SystemDataManager::getPlatformSummaryFilePath() const
{
    return m_platform_summary_file_path;
}

} // namespace matisse


