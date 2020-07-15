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

QString MatissePreferences::remoteServerAddress() const
{
    return _remoteServerAddress;
}

void MatissePreferences::setRemoteServerAddress(const QString& remoteServerAddress)
{
    _remoteServerAddress = remoteServerAddress;
}

QString MatissePreferences::remoteUsername() const
{
    return _remoteUsername;
}

void MatissePreferences::setRemoteUsername(const QString& remoteUsername)
{
    _remoteUsername = remoteUsername;
}

QString MatissePreferences::remoteQueueName() const
{
    return _remoteQueueName;
}

void MatissePreferences::setRemoteQueueName(const QString& remoteQueueName)
{
    _remoteQueueName = remoteQueueName;
}

QString MatissePreferences::remoteDefaultDataPath() const
{
    return _remoteDefaultDataPath;
}

void MatissePreferences::setRemoteDefaultDataPath(const QString& remoteDefaultDataPath)
{
    _remoteDefaultDataPath = remoteDefaultDataPath;
}

QString MatissePreferences::remoteResultPath() const
{
    return _remoteResultPath;
}

void MatissePreferences::setRemoteResultPath(const QString& remoteResultPath)
{
    _remoteResultPath = remoteResultPath;
}

MatissePreferences::MatissePreferences()
{
    
}

