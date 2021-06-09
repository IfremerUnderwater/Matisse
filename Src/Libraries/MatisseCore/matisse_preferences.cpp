#include "matisse_preferences.h"

namespace matisse {

QString MatissePreferences::importExportPath() const
{
    return m_import_export_path;
}

void MatissePreferences::setImportExportPath(const QString &_import_export_path)
{
    m_import_export_path = _import_export_path;
}

QString MatissePreferences::archivePath() const
{
    return m_archive_path;
}

void MatissePreferences::setArchivePath(const QString &_archive_path)
{
    m_archive_path = _archive_path;
}

QString MatissePreferences::defaultResultPath() const
{
    return m_default_result_path;
}

void MatissePreferences::setDefaultResultPath(const QString &_default_result_path)
{
    m_default_result_path = _default_result_path;
}

QString MatissePreferences::defaultMosaicFilenamePrefix() const
{
    return m_default_mosaic_filename_prefix;
}

void MatissePreferences::setDefaultMosaicFilenamePrefix(const QString &_default_mosaic_filename_prefix)
{
    m_default_mosaic_filename_prefix = _default_mosaic_filename_prefix;
}

bool MatissePreferences::programmingModeEnabled() const
{
    return m_programming_mode_enabled;
}

void MatissePreferences::setProgrammingModeEnabled(bool _programming_mode_enabled)
{
    m_programming_mode_enabled = _programming_mode_enabled;
}

QString MatissePreferences::language() const
{
    return m_language;
}

void MatissePreferences::setLanguage(const QString &_language)
{
    m_language = _language;
}

QDateTime MatissePreferences::lastUpdate() const
{
    return m_last_update;
}

void MatissePreferences::setLastUpdate(const QDateTime &_last_update)
{
    m_last_update = _last_update;
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

MatissePreferences::MatissePreferences()
{
    
}

} // namespace matisse
