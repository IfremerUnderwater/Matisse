#ifndef MATISSE_SSH_ACTION_H_
#define MATISSE_SSH_ACTION_H_

#include <QObject>

#include "SshClient.h"

namespace MatisseCommon {

	class SshActionManager;

	class SshAction : public QObject
	{	
		Q_OBJECT

	public:
		enum class SshActionType { UploadFile, DownloadFile, UploadDir, DownloadDir, SendCommand };
		Q_ENUM(SshActionType)

		SshAction(SshActionManager *_manager, SshActionType _type);

		SshActionType type() { return m_type; }
		bool isValid() { return m_is_valid; }
    bool isTerminated() { return m_is_terminated; }

		virtual void init() = 0;
		virtual void execute() = 0;
    void terminate();

	protected:
    virtual void doTerminate()=0;
		
		bool m_is_valid = true;
		bool m_is_terminated = false;
		SshActionManager *m_manager;

	private:
		SshActionType m_type;
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

	protected:
    void doTerminate();

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

	protected:
		void doTerminate();

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

	protected:
    void doTerminate();

  private:
		QString _command;
		QString _response;
	};

}

#endif  // MATISSE_SSH_ACTION_H_