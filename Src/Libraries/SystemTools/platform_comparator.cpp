#include "platform_comparator.h"

namespace system_tools {

PlatformComparator::PlatformComparator() :
    m_status(NULL)
{
}

PlatformComparator::~PlatformComparator()
{
    if (m_status) {
        delete m_status;
    }
}

void PlatformComparator::compareElementVersions(QString _local_version, QString _remote_version, PlatformElementCompare &_version_compare)
{
    // TODO verifier le format de version avec une expression reguliere

    if (_remote_version == _local_version) {
        qDebug() << "Local and remote versions match";
        _version_compare.m_does_match = true;
        _version_compare.m_version_compare = SAME;
    } else {
        _version_compare.m_does_match = false;
        _version_compare.m_version_compare = DIFFERENT; // if comparison is not possible

        QStringList local_version_parts = _local_version.split(".");
        QStringList remote_version_parts = _remote_version.split(".");

        // compare on as many version items are present on both parts
        int version_comparison_scope = qMin(local_version_parts.size(), remote_version_parts.size());

        bool found_difference = false;

        for (int i=0 ; i < version_comparison_scope ; i++) {
            QString local = local_version_parts.at(i);
            QString remote = remote_version_parts.at(i);
            int compared = remote.compare(local);

            if (compared == 0) {
                // same value, continue
                continue;
            }

            if (compared > 0) {
                // remote is newer than local
                qDebug() << "Remote version is newer than local version";
                _version_compare.m_version_compare = NEWER;
                found_difference = true;
                break;
            } else {
                // remote is older than local
                qDebug() << "Remote version is older than local version";
                _version_compare.m_version_compare = OLDER;
                found_difference = true;
                break;
            }

            if (!found_difference) {
                if (local_version_parts.size() == remote_version_parts.size()) {
                    qWarning() << QString("Local and remote versions are identical although their version litteral are different:\nLocal: %1\nRemote: %2")
                                .arg(_local_version).arg(_remote_version);
                    _version_compare.m_does_match = true;
                    _version_compare.m_version_compare = SAME;
                } else {
                    qWarning() << QString("Local and remote versions could not be compared. Will be considered different");
                }

            }

        }

    }
}

PlatformComparisonStatus *PlatformComparator::compare(PlatformDump *_local_platform, PlatformDump *_remote_platform)
{
    if (m_status) {
    // Delete status of previous comparison
        delete m_status;
    }

    m_status = new PlatformComparisonStatus();

    if (!_local_platform) {
        qCritical() << "The local platform dump is null. Could not compare local and remote platforms.";
        return m_status;
    }

    if (!_remote_platform) {
        qCritical() << "The remote platform dump is null. Could not compare local and remote platforms.";
        return m_status;
    }

    /* COMPARE MATISSE VERSIONS */
    bool matisse_versions_match = true;

    QString local_matisse_version = _local_platform->getMatisseVersion().trimmed();
    QString remote_matisse_version = _remote_platform->getMatisseVersion().trimmed();

    qDebug() << "Comparing Matisse versions...";
    PlatformElementCompare matisse_version_compare;
    compareElementVersions(local_matisse_version, remote_matisse_version, matisse_version_compare);
    m_status->setMatisseCompared(matisse_version_compare);

    matisse_versions_match = matisse_version_compare.m_does_match;

    /* COMPARE OS */
    bool os_match = true;

    QString local_os_name = _local_platform->getOsName().trimmed();
    QString remote_os_name = _remote_platform->getOsName().trimmed();

    qDebug() << "Comparing OS...";
    PlatformElementCompare os_compare;

    if (remote_os_name != local_os_name) {
        qWarning() << QString("Remote and local operating systems are different.\nLocal: %1\nRemote: %2")
                      .arg(remote_os_name).arg(local_os_name);
        os_compare.m_is_same_type = false;
        os_compare.m_does_match = false;
        os_compare.m_version_compare = DIFFERENT;
        os_match = false;

        QString platform_gap_message = tr("OS differents. Pf distant: %1 --- Pf local: %2");
        platform_gap_message.arg(remote_os_name).arg(local_os_name);
        m_status->addPlaformGapMessage(platform_gap_message);
    } else {
        QString local_os_version = _local_platform->getOsName();
        QString remote_os_version = _remote_platform->getOsName();

        // versions comparison is limited to versions litteral comparison
        if (local_os_version == remote_os_version) {
            qDebug() << "Remote and local operating systems have the same version.";
            os_compare.m_does_match = true;
            os_compare.m_version_compare = SAME;
        } else {
            qWarning() << QString("Remote and local operating systems have different versions.\nLocal: %1\nRemote: %2")
                          .arg(remote_os_version).arg(local_os_version);
            os_compare.m_does_match = false;
            os_compare.m_version_compare = DIFFERENT;
            os_match = false;

            QString platformGapMessage = tr("OS version differents. Pf distant: %1 --- Pf local: %2");
            platformGapMessage.arg(remote_os_version).arg(local_os_version);
            m_status->addPlaformGapMessage(platformGapMessage);
        }
    }

    m_status->setOsCompared(os_compare);

    /* COMPARE COMPONENTS */
    bool components_match = true;

    qDebug() << "Comparing dependencies...";

    QMap<QString, QString> *local_components_info = _local_platform->getComponentsInfo();
    QMap<QString, QString> *remote_components_info = _remote_platform->getComponentsInfo();

    int local_components_nb = local_components_info->size();
    int remote_components_nb = remote_components_info->size();

    if (remote_components_nb != local_components_nb) {
        qWarning() << QString("Remote and local platforms have a different number of dependencies.\nRemote: %1\nLocal: %2")
                      .arg(remote_components_nb).arg(local_components_nb);
    }

    // to identify dependencies found in remote but not in local platform
    QList<QString> extra_remote_components = remote_components_info->keys();

    foreach (QString componentName, local_components_info->keys()) {
        PlatformElementCompare component_compare;

        if (remote_components_info->contains(componentName)) {
            extra_remote_components.removeOne(componentName);
            QString local_component_version = local_components_info->value(componentName);
            QString remote_component_version = remote_components_info->value(componentName);

            qDebug() << QString("Comparing versions for component '%1'...").arg(componentName);
            compareElementVersions(local_component_version, remote_component_version, component_compare);

            if (!component_compare.m_does_match) {
                components_match = false;

                QString platform_gap_message = tr("Different versions for component '%1'. Pf distant: %2 --- Pf local: %3");
                platform_gap_message.arg(componentName).arg(remote_component_version).arg(local_component_version);
                m_status->addPlaformGapMessage(platform_gap_message);
            }
        } else {
            qWarning() << QString("Dependency '%1' is missing from remote platform").arg(componentName);
            component_compare.m_does_match = false;
            component_compare.m_found_in_remote_platform = false;

            /* A missing dependency in remote platform can be the source of a silent process failure.
             * For this reason, it is considered as platform mismatch */
            components_match = false;

            QString platform_gap_message = tr("Component '%1' not present on distant platform.");
            platform_gap_message.arg(componentName);
            m_status->addPlaformGapMessage(platform_gap_message);
        }

        m_status->addComponentCompared(componentName, component_compare);
    }

    if (!extra_remote_components.isEmpty()) {
        foreach (QString remoteComponentName, extra_remote_components) {
            qWarning() << QString("Dependency '%1' is found in remote platform but not in local platform").arg(remoteComponentName);

            PlatformElementCompare component_compare;
            component_compare.m_does_match = false;
            component_compare.m_found_in_local_platform = false;
            m_status->addComponentCompared(remoteComponentName, component_compare);

            QString platform_gap_message = tr("Component '%1' present on distant platform but absent on local one.");
            platform_gap_message.arg(remoteComponentName);
            m_status->addPlaformGapMessage(platform_gap_message);
        }

        /* If remote platform has extra dependencies but all dependencies found in local platform match,
         * it is not likely a source of error but can indicate that the remote platform configuration is obsolete
         * ==> marked as mismatch */
        components_match = false;
    }

    /* Evaluate global match */
    bool platforms_match = matisse_versions_match && os_match && components_match;
    m_status->setDoesMatch(platforms_match);

    return m_status;
}

} // namespace system_tools

