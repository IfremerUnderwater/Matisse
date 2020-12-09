#include "MatisseRemoteServerSettings.h"

namespace MatisseTools {

RemoteServerSetting::RemoteServerSetting() {
}

RemoteServerSetting::RemoteServerSetting(const RemoteServerSetting& _setting) {
  m_path = _setting.m_path;
  m_alias = _setting.m_alias;
}


QString RemoteServerSetting::path() const { 
	return m_path; 
}
	
void RemoteServerSetting::setPath(const QString& _path) { 
	m_path = _path; 
}

QString RemoteServerSetting::alias() const { 
	return m_alias; 
}
	
void RemoteServerSetting::setAlias(const QString& _alias) { 
	m_alias = _alias; 
}
	


RemoteServerSetting MatisseRemoteServerSettings::containerImage() const {
  return m_container_image;
}

void MatisseRemoteServerSettings::setContainerImage(const RemoteServerSetting& _container_image) {
  m_container_image = _container_image;
}

RemoteServerSetting MatisseRemoteServerSettings::launcherParentDir() const {
  return m_launcher_parent_dir;
}

void MatisseRemoteServerSettings::setLauncherParentDir(const RemoteServerSetting& _launcher_parent_dir) {
  m_launcher_parent_dir = _launcher_parent_dir;
}

RemoteServerSetting MatisseRemoteServerSettings::launcherFile() const {
  return m_launcher_file; 
}

void MatisseRemoteServerSettings::setLauncherFile(const RemoteServerSetting& _launcher_file) {
  m_launcher_file = _launcher_file;
}

RemoteServerSetting MatisseRemoteServerSettings::binRoot() const { 
  return m_bin_root;
}

void MatisseRemoteServerSettings::setBinRoot(const RemoteServerSetting& _bin_root) {
  m_bin_root = _bin_root;
}

RemoteServerSetting MatisseRemoteServerSettings::applicationFilesRoot() const {
  return m_application_files_root;
}

void MatisseRemoteServerSettings::setApplicationFilesRoot(const RemoteServerSetting& _application_files_root) {
  m_application_files_root = _application_files_root;
}

RemoteServerSetting MatisseRemoteServerSettings::datasetsSubdir() const {
  return m_datasets_subdir;
}

void MatisseRemoteServerSettings::setDatasetsSubdir(const RemoteServerSetting& _datasets_subdir) {
  m_datasets_subdir = _datasets_subdir;
}

RemoteServerSetting MatisseRemoteServerSettings::jobsSubdir() const { 
  return m_jobs_subdir; 
}

void MatisseRemoteServerSettings::setJobsSubdir(const RemoteServerSetting& _jobs_subdir) {
  m_jobs_subdir = _jobs_subdir;
}

RemoteServerSetting MatisseRemoteServerSettings::resultsSubdir() const { 
  return m_results_subdir; 
}

void MatisseRemoteServerSettings::setResultsSubdir(const RemoteServerSetting& _results_subdir) {
  m_results_subdir = _results_subdir;
}

QString MatisseRemoteServerSettings::datasetsPath() {
  QString datasets_path = applicationFilesRoot().path() + '/' + datasetsSubdir().path();
  return datasets_path;
}

QString MatisseRemoteServerSettings::datasetsPathBound() { 
  QString datasets_path_bound = "~/" + datasetsSubdir().alias();
  return datasets_path_bound;
}

QString MatisseRemoteServerSettings::jobsPath() { 
  QString jobs_path = applicationFilesRoot().path() + '/' + jobsSubdir().path();
  return jobs_path;
}

QString MatisseRemoteServerSettings::jobsPathBound() { 
  QString jobs_path_bound = "~/" + jobsSubdir().alias();
  return jobs_path_bound; 
}

QString MatisseRemoteServerSettings::resultsPath() { 
  QString results_path = applicationFilesRoot().path() + '/' + resultsSubdir().path();
  return results_path;
}

QString MatisseRemoteServerSettings::resultsPathBound() { 
  QString results_path_bound = "~/" + resultsSubdir().alias();
  return results_path_bound;
}

}  // namespace MatisseTools