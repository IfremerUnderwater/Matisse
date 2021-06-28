#ifndef MATISSE_MATISSE_PREFERENCES_H_
#define MATISSE_MATISSE_PREFERENCES_H_

#include <QObject>
#include <QtGlobal>
#include <QString>
#include <QDateTime>

namespace matisse {

///
/// \brief The MatissePreferences class is the business class for Matisse preferences
///

class MatissePreferences
{
private:
   QDateTime m_last_update;
   QString m_import_export_path;
   QString m_archive_path;
   QString m_default_result_path;
   QString m_default_mosaic_filename_prefix;
   bool m_programming_mode_enabled;
   QString m_language;

   /* Remote execution preferences */
   QString m_remote_command_server;
   QString m_remote_file_server;
   QString m_remote_username;
   QString m_remote_user_email;
   QString m_remote_queue_name;
   int m_remote_nb_of_cpus;

public:
    MatissePreferences();
    QString importExportPath() const;
    void setImportExportPath(const QString &_import_export_path);
    QString archivePath() const;
    void setArchivePath(const QString &_archive_path);
    QString defaultResultPath() const;
    void setDefaultResultPath(const QString &_default_result_path);
    QString defaultMosaicFilenamePrefix() const;
    void setDefaultMosaicFilenamePrefix(const QString &_default_mosaic_filename_prefix);
    bool programmingModeEnabled() const;
    void setProgrammingModeEnabled(bool _programming_mode_enabled);
    QString language() const;
    void setLanguage(const QString &_language);
    QDateTime lastUpdate() const;
    void setLastUpdate(const QDateTime &_last_update);

    QString remoteCommandServer() const;
    void setRemoteCommandServer(const QString& _remote_command_server);
    QString remoteFileServer() const;
    void setRemoteFileServer(const QString& _remote_file_server);
    QString remoteUsername() const;
    void setRemoteUsername(const QString& _remote_username);    
    QString remoteUserEmail() const;
    void setRemoteUserEmail(const QString& _remote_user_email);
    QString remoteQueueName() const;
    void setRemoteQueueName(const QString& _remote_queue_name);
    int remoteNbOfCpus() const;
    void setRemoteNbOfCpus(const int _remote_nb_of_cpus);
};

} // namespace matisse

#endif // MATISSE_MATISSE_PREFERENCES_H_
