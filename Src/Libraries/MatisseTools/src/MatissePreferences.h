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
};

#endif // MATISSEPREFERENCES_H
