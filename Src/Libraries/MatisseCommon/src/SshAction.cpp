#include "SshAction.h"

#include <QtDebug>
#include <QFileInfo>

using namespace MatisseCommon;

SshAction::SshAction(SshActionManager* manager, SshActionType type)
{
	_manager = manager;
	_type = type;
}


UploadFileAction::UploadFileAction(SshActionManager* manager, QString localFilePath, QString remotePath) :
	SshAction(manager, SshActionType::UploadFile)
{
	QFileInfo info(localFilePath);

	if (!info.exists()) {
		qCritical() << QString("QSshClient: %1 cannot be uploaded : file does not exist").arg(localFilePath);
		_isValid = false;
	}

	_localFilePath = info.canonicalFilePath();
	_remotePath = remotePath + QLatin1Char('/') + info.fileName();
}

void UploadFileAction::init() {
	_manager->createSftpChannel();
}

void UploadFileAction::execute() {
	_manager->upload(_localFilePath, _remotePath);
}

UploadDirAction::UploadDirAction(SshActionManager* manager, QString localDir, QString remoteBaseDir) :
	SshAction(manager, SshActionType::UploadDir)
{
	QFileInfo info(localDir);

	if (!info.exists()) {
		qCritical() << QString("QSshClient: %1 cannot be uploaded : dir does not exist").arg(localDir);
		_isValid = false;
	}

	_localDir = info.canonicalFilePath();
	_remoteBaseDir = remoteBaseDir;
}

void UploadDirAction::init() {
	_manager->createSftpChannel();
}

void UploadDirAction::execute() {
	_manager->upload(_localDir, _remoteBaseDir, true);
}

SendCommandAction::SendCommandAction(SshActionManager* manager, QString commandString) :
	SshAction(manager, SshActionType::SendCommand), 
	_command(commandString)
{
}

void SendCommandAction::init() {
	qDebug() << "SendCommandAction init";
	_manager->createRemoteShell(_command);
}

void SendCommandAction::execute() {
	qDebug() << "SendCommandAction execute";
	_manager->executeCommand();
}
