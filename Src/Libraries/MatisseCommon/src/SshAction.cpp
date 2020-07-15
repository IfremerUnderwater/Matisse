#include "SshAction.h"

#include <QtDebug>
#include <QFileInfo>

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

  _localFilePath = info.canonicalFilePath();
  _remotePath = remotePath + QLatin1Char('/') + info.fileName();
}

void UploadFileAction::init() { m_manager->createSftpChannel(); }

void UploadFileAction::execute() {
  m_manager->upload(_localFilePath, _remotePath);
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

  _localDir = info.canonicalFilePath();
  _remoteBaseDir = remoteBaseDir;
}

void UploadDirAction::init() { m_manager->createSftpChannel(); }

void UploadDirAction::execute() {
  m_manager->upload(_localDir, _remoteBaseDir, true);
}

void UploadDirAction::doTerminate() {}

SendCommandAction::SendCommandAction(SshActionManager* manager,
                                     QString commandString)
    : SshAction(manager, SshActionType::SendCommand), _command(commandString) {}

void SendCommandAction::init() {
  qDebug() << "SendCommandAction init";
  m_manager->createRemoteShell(_command);
}

void SendCommandAction::execute() {
  qDebug() << "SendCommandAction execute";
  m_manager->executeCommand();
}

void SendCommandAction::doTerminate() {
  qDebug() << "SendCommandAction closing remote shell";
  m_manager->closeRemoteShell();
}

}  // namespace MatisseCommon