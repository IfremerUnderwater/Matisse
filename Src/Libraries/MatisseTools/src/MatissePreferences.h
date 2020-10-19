#ifndef MATISSEPREFERENCES_H
#define MATISSEPREFERENCES_H

#include <QObject>
#include <QtGlobal>
#include <QString>
#include <QDateTime>

///
/// \brief The MatissePreferences class is the business class for Matisse preferences
///

class MatissePreferences
{
private:
   QDateTime _lastUpdate;
   QString _importExportPath;
   QString _archivePath;
   QString _defaultResultPath;
   QString _defaultMosaicFilenamePrefix;
   bool _programmingModeEnabled;
   QString _language;

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
    void setImportExportPath(const QString &importExportPath);
    QString archivePath() const;
    void setArchivePath(const QString &archivePath);
    QString defaultResultPath() const;
    void setDefaultResultPath(const QString &defaultResultPath);
    QString defaultMosaicFilenamePrefix() const;
    void setDefaultMosaicFilenamePrefix(const QString &defaultMosaicFilenamePrefix);
    bool programmingModeEnabled() const;
    void setProgrammingModeEnabled(bool programmingModeEnabled);
    QString language() const;
    void setLanguage(const QString &language);
    QDateTime lastUpdate() const;
    void setLastUpdate(const QDateTime &lastUpdate);

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

#endif // MATISSEPREFERENCES_H
