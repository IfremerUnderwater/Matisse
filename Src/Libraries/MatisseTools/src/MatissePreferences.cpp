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
MatissePreferences::MatissePreferences()
{
    
}

