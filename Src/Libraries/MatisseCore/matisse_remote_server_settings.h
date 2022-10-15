#ifndef MATISSE_REMOTE_SERVER_SETTINGS_H
#define MATISSE_REMOTE_SERVER_SETTINGS_H

#include <QObject>

namespace matisse {

class RemoteServerSetting {
public:
    RemoteServerSetting();
    RemoteServerSetting(const RemoteServerSetting& _setting);

    QString path() const;
    void setPath(const QString& _path);

    QString alias() const;
    void setAlias(const QString& _alias);

private:
    QString m_path;   /// remote server OS path to specific resource (directory or
    /// file)
    QString m_alias;  /// container alias to same resource (only applicable to
    /// directories)
};

class MatisseRemoteServerSettings : public QObject {
public:
    MatisseRemoteServerSettings() {}

    RemoteServerSetting containerImage() const;
    void setContainerImage(const RemoteServerSetting& _container_image);

    RemoteServerSetting launcherParentDir() const;
    void setLauncherParentDir(const RemoteServerSetting& _launcher_parent_dir);

    RemoteServerSetting launcherFile() const;
    void setLauncherFile(const RemoteServerSetting& _launcher_file);

    RemoteServerSetting binRoot() const;
    void setBinRoot(const RemoteServerSetting& _bin_root);

    RemoteServerSetting applicationFilesRoot() const;
    void setApplicationFilesRoot(
            const RemoteServerSetting& _application_files_root);

    RemoteServerSetting datasetsDir() const;
    void setDatasetsDir(const RemoteServerSetting& _datasets_dir);

    RemoteServerSetting jobsSubdir() const;
    void setJobsSubdir(const RemoteServerSetting& _jobs_subdir);

    RemoteServerSetting resultsSubdir() const;
    void setResultsSubdir(const RemoteServerSetting& _results_subdir);

    /* Shortcuts to paths and aliases for application folders */
    QString datasetsPath();
    QString datasetsPathBound();
    QString jobsPath();
    QString jobsPathBound();
    QString resultsPath();
    QString resultsPathBound();

private:
    RemoteServerSetting m_container_image;
    RemoteServerSetting m_launcher_parent_dir;
    RemoteServerSetting m_launcher_file;
    RemoteServerSetting m_bin_root;
    RemoteServerSetting m_application_files_root;
    RemoteServerSetting m_datasets_dir;
    RemoteServerSetting m_jobs_subdir;
    RemoteServerSetting m_results_subdir;
};

} // namespace matisse

#endif // MATISSE_REMOTE_SERVER_SETTINGS_H
