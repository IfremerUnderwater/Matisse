#include "PlatformComparator.h"

using namespace MatisseTools;

PlatformComparator::PlatformComparator() :
    _status(NULL)
{
}

PlatformComparator::~PlatformComparator()
{
    if (_status) {
        delete _status;
    }
}

void PlatformComparator::compareElementVersions(QString localVersion, QString remoteVersion, PlatformElementCompare &versionCompare)
{
    // TODO verifier le format de version avec une expression reguliere

    if (remoteVersion == localVersion) {
        qDebug() << "Local and remote versions match";
        versionCompare._doesMatch = true;
        versionCompare._versionCompare = SAME;
    } else {
        versionCompare._doesMatch = false;
        versionCompare._versionCompare = DIFFERENT; // if comparison is not possible

        QStringList localVersionParts = localVersion.split(".");
        QStringList remoteVersionParts = remoteVersion.split(".");

        // compare on as many version items are present on both parts
        int versionComparisonScope = qMin(localVersionParts.size(), remoteVersionParts.size());

        bool foundDifference = false;

        for (int i=0 ; i++ ; i < versionComparisonScope) {
            QString local = localVersionParts.at(i);
            QString remote = remoteVersionParts.at(i);
            int compared = remote.compare(local);

            if (compared == 0) {
                // same value, continue
                continue;
            }

            if (compared > 0) {
                // remote is newer than local
                qDebug() << "Remote version is newer than local version";
                versionCompare._versionCompare = NEWER;
                foundDifference = true;
                break;
            } else {
                // remote is older than local
                qDebug() << "Remote version is older than local version";
                versionCompare._versionCompare = OLDER;
                foundDifference = true;
                break;
            }

            if (!foundDifference) {
                if (localVersionParts.size() == remoteVersionParts.size()) {
                    qWarning() << QString("Local and remote versions are identical although their version litteral are different:\nLocal: %1\nRemote: %2")
                                .arg(localVersion).arg(remoteVersion);
                    versionCompare._doesMatch = true;
                    versionCompare._versionCompare = SAME;
                } else {
                    qWarning() << QString("Local and remote versions could not be compared. Will be considered different");
                }

            }

        }

    }
}

PlatformComparisonStatus *PlatformComparator::compare(PlatformDump *localPlatform, PlatformDump *remotePlatform)
{
    if (_status) {
    // Delete status of previous comparison
        delete _status;
    }

    _status = new PlatformComparisonStatus();

    if (!localPlatform) {
        qCritical() << "The local platform dump is null. Could not compare local and remote platforms.";
        return _status;
    }

    if (!remotePlatform) {
        qCritical() << "The remote platform dump is null. Could not compare local and remote platforms.";
        return _status;
    }

    /* COMPARE MATISSE VERSIONS */
    bool matisseVersionsMatch = true;

    QString localMatisseVersion = localPlatform->getMatisseVersion().trimmed();
    QString remoteMatisseVersion = remotePlatform->getMatisseVersion().trimmed();

    qDebug() << "Comparing Matisse versions...";
    PlatformElementCompare matisseVersionCompare;
    compareElementVersions(localMatisseVersion, remoteMatisseVersion, matisseVersionCompare);
    _status->setMatisseCompared(matisseVersionCompare);

    matisseVersionsMatch = matisseVersionCompare._doesMatch;

    /* COMPARE OS */
    bool osMatch = true;

    QString localOsName = localPlatform->getOsName().trimmed();
    QString remoteOsName = remotePlatform->getOsName().trimmed();

    qDebug() << "Comparing OS...";
    PlatformElementCompare osCompare;

    if (remoteOsName != localOsName) {
        qWarning() << QString("Remote and local operating systems are different.\nLocal: %1\nRemote: %2")
                      .arg(remoteOsName).arg(localOsName);
        osCompare._isSameType = false;
        osCompare._doesMatch = false;
        osCompare._versionCompare = DIFFERENT;
        osMatch = false;

        QString platformGapMessage = tr("Types d'OS differents. Pf distante: %1 --- Pf locale: %2");
        platformGapMessage.arg(remoteOsName).arg(localOsName);
        _status->addPlaformGapMessage(platformGapMessage);
    } else {
        QString localOsVersion = localPlatform->getOsName();
        QString remoteOsVersion = remotePlatform->getOsName();

        // versions comparison is limited to versions litteral comparison
        if (localOsVersion == remoteOsVersion) {
            qDebug() << "Remote and local operating systems have the same version.";
            osCompare._doesMatch = true;
            osCompare._versionCompare = SAME;
        } else {
            qWarning() << QString("Remote and local operating systems have different versions.\nLocal: %1\nRemote: %2")
                          .arg(remoteOsVersion).arg(localOsVersion);
            osCompare._doesMatch = false;
            osCompare._versionCompare = DIFFERENT;
            osMatch = false;

            QString platformGapMessage = tr("Versions d'OS differentes. Pf distante: %1 --- Pf locale: %2");
            platformGapMessage.arg(remoteOsVersion).arg(localOsVersion);
            _status->addPlaformGapMessage(platformGapMessage);
        }
    }

    _status->setOsCompared(osCompare);

    /* COMPARE COMPONENTS */
    bool componentsMatch = true;

    qDebug() << "Comparing dependencies...";

    QMap<QString, QString> *localComponentsInfo = localPlatform->getComponentsInfo();
    QMap<QString, QString> *remoteComponentsInfo = remotePlatform->getComponentsInfo();

    int localComponentsNb = localComponentsInfo->size();
    int remoteComponentsNb = remoteComponentsInfo->size();

    if (remoteComponentsNb != localComponentsNb) {
        qWarning() << QString("Remote and local platforms have a different number of dependencies.\nRemote: %1\nLocal: %2")
                      .arg(remoteComponentsNb).arg(localComponentsNb);
    }

    // to identify dependencies found in remote but not in local platform
    QList<QString> extraRemoteComponents = remoteComponentsInfo->keys();

    foreach (QString componentName, localComponentsInfo->keys()) {
        PlatformElementCompare componentCompare;

        if (remoteComponentsInfo->contains(componentName)) {
            extraRemoteComponents.removeOne(componentName);
            QString localComponentVersion = localComponentsInfo->value(componentName);
            QString remoteComponentVersion = remoteComponentsInfo->value(componentName);

            qDebug() << QString("Comparing versions for component '%1'...").arg(componentName);
            compareElementVersions(localComponentVersion, remoteComponentVersion, componentCompare);

            if (!componentCompare._doesMatch) {
                componentsMatch = false;

                QString platformGapMessage = tr("Versions differentes pour le composant '%1'. Pf distante: %2 --- Pf locale: %3");
                platformGapMessage.arg(componentName).arg(remoteComponentVersion).arg(localComponentVersion);
                _status->addPlaformGapMessage(platformGapMessage);
            }
        } else {
            qWarning() << QString("Dependency '%1' is missing from remote platform").arg(componentName);
            componentCompare._doesMatch = false;
            componentCompare._foundInRemotePlatform = false;

            /* A missing dependency in remote platform can be the source of a silent process failure.
             * For this reason, it is considered as platform mismatch */
            componentsMatch = false;

            QString platformGapMessage = tr("Composant '%1' absent de la plateforme distante.");
            platformGapMessage.arg(componentName);
            _status->addPlaformGapMessage(platformGapMessage);
        }

        _status->addComponentCompared(componentName, componentCompare);
    }

    if (!extraRemoteComponents.isEmpty()) {
        foreach (QString remoteComponentName, extraRemoteComponents) {
            qWarning() << QString("Dependency '%1' is found in remote platform but not in local platform").arg(remoteComponentName);

            PlatformElementCompare componentCompare;
            componentCompare._doesMatch = false;
            componentCompare._foundInLocalPlatform = false;
            _status->addComponentCompared(remoteComponentName, componentCompare);

            QString platformGapMessage = tr("Composant '%1' present sur la plateforme distante mais absent de la plateforme locale.");
            platformGapMessage.arg(remoteComponentName);
            _status->addPlaformGapMessage(platformGapMessage);
        }

        /* If remote platform has extra dependencies but all dependencies found in local platform match,
         * it is not likely a source of error but can indicate that the remote platform configuration is obsolete
         * ==> marked as mismatch */
        componentsMatch = false;
    }

    /* Evaluate global match */
    bool platformsMatch = matisseVersionsMatch && osMatch && componentsMatch;
    _status->setDoesMatch(platformsMatch);

    return _status;
}

