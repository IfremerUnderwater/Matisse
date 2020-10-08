#include "MatissePreferences.h"


QString MatissePreferences::importExportPath() const
{
    return _importExportPath;
}

void MatissePreferences::setImportExportPath(const QString &importExportPath)
{
    _importExportPath = importExportPath;
}

QString MatissePreferences::archivePath() const
{
    return _archivePath;
}

void MatissePreferences::setArchivePath(const QString &archivePath)
{
    _archivePath = archivePath;
}

QString MatissePreferences::defaultResultPath() const
{
    return _defaultResultPath;
}

void MatissePreferences::setDefaultResultPath(const QString &defaultResultPath)
{
    _defaultResultPath = defaultResultPath;
}

QString MatissePreferences::defaultMosaicFilenamePrefix() const
{
    return _defaultMosaicFilenamePrefix;
}

void MatissePreferences::setDefaultMosaicFilenamePrefix(const QString &defaultMosaicFilenamePrefix)
{
    _defaultMosaicFilenamePrefix = defaultMosaicFilenamePrefix;
}

bool MatissePreferences::programmingModeEnabled() const
{
    return _programmingModeEnabled;
}

void MatissePreferences::setProgrammingModeEnabled(bool programmingModeEnabled)
{
    _programmingModeEnabled = programmingModeEnabled;
}

QString MatissePreferences::language() const
{
    return _language;
}

void MatissePreferences::setLanguage(const QString &language)
{
    _language = language;
}

QDateTime MatissePreferences::lastUpdate() const
{
    return _lastUpdate;
}

void MatissePreferences::setLastUpdate(const QDateTime &lastUpdate)
{
    _lastUpdate = lastUpdate;
}

QString MatissePreferences::remoteCommandServer() const
{
    return m_remote_command_server;
}

void MatissePreferences::setRemoteCommandServer(const QString& _remote_server_address)
{
    m_remote_command_server = _remote_server_address;
}

QString MatissePreferences::remoteFileServer() const {
  return m_remote_file_server;
}

void MatissePreferences::setRemoteFileServer(
    const QString &_remote_file_server) 
{
  m_remote_file_server = _remote_file_server;
}

QString MatissePreferences::remoteUsername() const
{
    return m_remote_username;
}

void MatissePreferences::setRemoteUsername(const QString& _remote_username)
{
    m_remote_username = _remote_username;
}

QString MatissePreferences::remoteUserEmail() const 
{ 
  return m_remote_user_email; 
}

void MatissePreferences::setRemoteUserEmail(const QString &_remote_user_email) 
{
  m_remote_user_email = _remote_user_email;
}

QString MatissePreferences::remoteQueueName() const
{
    return m_remote_queue_name;
}

void MatissePreferences::setRemoteQueueName(const QString& _remote_queue_name)
{
    m_remote_queue_name = _remote_queue_name;
}

int MatissePreferences::remoteNbOfCpus() const 
{ 
  return m_remote_nb_of_cpus; 
}

void MatissePreferences::setRemoteNbOfCpus(const int _remote_nb_of_cpus) 
{
  m_remote_nb_of_cpus = _remote_nb_of_cpus;
}

QString MatissePreferences::remoteDefaultDataPath() const
{
    return m_remote_default_data_path;
}

void MatissePreferences::setRemoteDefaultDataPath(const QString& _remote_default_data_path)
{
    m_remote_default_data_path = _remote_default_data_path;
}

QString MatissePreferences::remoteResultPath() const
{
    return m_remote_result_path;
}

void MatissePreferences::setRemoteResultPath(const QString& _remote_result_path)
{
    m_remote_result_path = _remote_result_path;
}

MatissePreferences::MatissePreferences()
{
    
}

