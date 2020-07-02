#pragma once

#include <QObject>
#include "SshClient.h"

namespace MatisseServer {

	class SshActionManager;

	class SshAction : public QObject
	{	
		Q_OBJECT

	public:
		enum class SshActionType { UploadFile, DownloadFile, UploadDir, DownloadDir, SendCommand };
		Q_ENUM(SshActionType)

		SshAction(SshActionManager *manager, SshActionType type);

		SshActionType type() { return _type; }
		bool isValid() { return _isValid; }

		virtual void init() = 0;
		virtual void execute() = 0;

	private:
		SshActionType _type;

	protected:
		bool _isValid = true;
		SshActionManager *_manager;


	};

	class UploadFileAction : public SshAction
	{
		//friend class SshActionManager;

	public:
		explicit UploadFileAction(SshActionManager* manager, QString localFilePath, QString remotePath);
		void init();
		void execute();
		QString localFilePath() { return _localFilePath; }
		QString remotePath() { return _remotePath; }

	private:
		QString _localFilePath;
		QString _remotePath;
	};

	class UploadDirAction : public SshAction
	{
		friend class SshActionManager;

	public:
		explicit UploadDirAction(SshActionManager* manager, QString localDir, QString remoteBaseDir);
		void init();
		void execute();
		QString localDir() { return _localDir; }
		QString remoteBaseDir() { return _remoteBaseDir; }

	private:
		QString _localDir;
		QString _remoteBaseDir;
	};
	
	class SendCommandAction : public SshAction
	{
		friend class SshActionManager;

	public:
		explicit SendCommandAction(SshActionManager* manager, QString commandString);
		void init();
		void execute();
		QString command() { return _command; }
		QString setResponse(QString response) { _response = response; }
		QString response() { return _response; }

	private:
		QString _command;
		QString _response;
	};

}

