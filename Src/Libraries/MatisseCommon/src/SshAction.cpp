#include "SshAction.h"

#include <QtDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace MatisseCommon {

SshAction::SshAction(SshActionManager* manager, SshActionType type) {
  m_manager = manager;
  m_type = type;
}

void SshAction::terminate() 
{ 
  m_is_terminated = true;

  /* Command specific termination */
  doTerminate();
}

UploadFileAction::UploadFileAction(SshActionManager* manager,
                                   QString localFilePath, QString remotePath)
    : SshAction(manager, SshActionType::UploadFile) {
  QFileInfo info(localFilePath);

  if (!info.exists()) {
    qCritical()
        << QString("QSshClient: %1 cannot be uploaded : file does not exist")
               .arg(localFilePath);
    m_is_valid = false;
  }

  m_local_file_path = info.canonicalFilePath();
  m_remote_path = remotePath + QLatin1Char('/') + info.fileName();
}

void UploadFileAction::init() { m_manager->createSftpChannel(); }

void UploadFileAction::execute() {
  m_manager->upload(m_local_file_path, m_remote_path);
}

void UploadFileAction::doTerminate() {}

UploadDirAction::UploadDirAction(SshActionManager* manager, QString localDir,
                                 QString remoteBaseDir)
    : SshAction(manager, SshActionType::UploadDir) {
  QFileInfo info(localDir);

  if (!info.exists()) {
    qCritical() << QString(
                       "QSshClient: %1 cannot be uploaded : dir does not exist")
                       .arg(localDir);
    m_is_valid = false;
  }

  m_local_dir = info.canonicalFilePath();
  m_remote_base_dir = remoteBaseDir;
}

void UploadDirAction::init() { m_manager->createSftpChannel(); }

void UploadDirAction::execute() {
  m_manager->upload(m_local_dir, m_remote_base_dir, true);
}

void UploadDirAction::doTerminate() {}


ListDirContentAction::ListDirContentAction(SshActionManager* manager,
  QString _remote_dir,
  FileTypeFilters _flags,
  QStringList _file_filters)
    : SshAction(manager, SshActionType::ListDirContent), m_file_filters() {

  m_remote_dir = _remote_dir;
  m_flags = _flags;

  /* check filters */
  for (QString filter : _file_filters) {
    QRegularExpression filter_rexp("\\*\\..+");
    QRegularExpressionMatch match = filter_rexp.match(filter);
    if (!match.hasMatch()) {
      qWarning() << QString(
                        "ListDirContentAction: filter '%1' is not in the "
                        "format '*.ext' and will be ignored")
                        .arg(filter);
      continue;
    }

    qDebug() << "ListDirContentAction: add file filter " << filter;
    m_file_filters.append(filter);
  }
}

void ListDirContentAction::init() { m_manager->createSftpChannel(); }

void ListDirContentAction::execute() {
  m_manager->dirContent(m_remote_dir, m_flags, m_file_filters);
}

void ListDirContentAction::doTerminate() {}




SendCommandAction::SendCommandAction(SshActionManager* manager,
                                     QString commandString)
    : SshAction(manager, SshActionType::SendCommand), m_command(commandString) {}

void SendCommandAction::init() {
  qDebug() << "SendCommandAction init";
  m_manager->createRemoteShell(m_command);
}

void SendCommandAction::execute() {
  qDebug() << "SendCommandAction execute";
  m_manager->executeCommand();
}

void SendCommandAction::doTerminate() {
  qDebug() << "SendCommandAction closing remote shell";
  m_manager->closeRemoteShell();
}

DownloadDirAction::DownloadDirAction(SshActionManager* _manager,
                                     QString _remote_dir,
                                     QString _local_base_dir) :
    SshAction(_manager, SshActionType::DownloadDir) 
{
  QFileInfo info(_local_base_dir);

  if (!info.exists()) {
    qCritical() << QString(
                       "DownloadDirAction: Remote dir cannot be downloaded to '%1' : target dir does not exist")
                       .arg(_local_base_dir);
    m_is_valid = false;
  }

  m_local_base_dir = info.canonicalFilePath();
  m_remote_dir = _remote_dir;
}

void DownloadDirAction::init() { m_manager->createSftpChannel(); }

void DownloadDirAction::execute() {
  m_manager->download(m_remote_dir, m_local_base_dir, true);
}

void DownloadDirAction::doTerminate() {}

}  // namespace MatisseCommon