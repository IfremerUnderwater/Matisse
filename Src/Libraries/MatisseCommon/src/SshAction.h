#ifndef MATISSE_SSH_ACTION_H_
#define MATISSE_SSH_ACTION_H_

#include <QDir>
#include <QObject>

namespace MatisseCommon {

enum class FileTypeFilter {
  Dirs = 0x001,
  Files = 0x002,
  AllEntries = Dirs | Files
};
Q_DECLARE_FLAGS(FileTypeFilters, FileTypeFilter)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileTypeFilters)

class SshActionManager : public QObject {
  friend class UploadFileAction;
  friend class UploadDirAction;
  friend class SendCommandAction;
  friend class ListDirContentAction;
  friend class DownloadDirAction;

  Q_OBJECT

 public:
  explicit SshActionManager();

 protected:
  virtual void createSftpChannel() = 0;
  virtual void createRemoteShell(QString& _command) = 0;
  virtual void closeRemoteShell() = 0;
  virtual void executeCommand() = 0;
  virtual void upload(QString _local_path, QString _remote_path,
                      bool _is_dir_upload = false) = 0;
  virtual void download(QString _remote_path, QString _local_path,
                        bool _is_dir_upload = false) = 0;
  virtual void dirContent(QString _remote_dir_path,
                          FileTypeFilters _flags = FileTypeFilter::AllEntries,
                          QStringList _file_filters = QStringList()) = 0;
};

	class SshAction : public QObject
	{	
		Q_OBJECT

	public:
		enum class SshActionType { UploadFile, DownloadFile, UploadDir, ListDirContent, DownloadDir, SendCommand };
		Q_ENUM(SshActionType)

		SshAction(SshActionManager *_manager, SshActionType _type);

		SshActionType type() { return m_type; }
		bool isValid() { return m_is_valid; }
    bool isTerminated() { return m_is_terminated; }
		QString metainfo() { return m_metainfo; }
    void setMetainfo(QString _metainfo) { m_metainfo = _metainfo; }

		virtual void init() = 0;
		virtual void execute() = 0;
		virtual QString progressMessage() = 0;
    void terminate();

	protected:
    virtual void doTerminate()=0;
		
		bool m_is_valid = true;
		bool m_is_terminated = false;
		SshActionManager *m_manager;
		QString m_metainfo;

	private:
		SshActionType m_type;
	};

	class UploadFileAction : public SshAction
	{
		// friend class SshActionManager;

	public:
		explicit UploadFileAction(SshActionManager* _manager, QString _local_file_path, QString _remote_path);
		void init();
		void execute();
		QString localFilePath() { return m_local_file_path; }
		QString remotePath() { return m_remote_path; }
		QString progressMessage() {
			return QString("Uploading file to '%1'").arg(m_remote_path); }

	protected:
    void doTerminate();

	private:
		QString m_local_file_path;
		QString m_remote_path;
	};

	class UploadDirAction : public SshAction
	{
		friend class SshActionManager;

	public:
		explicit UploadDirAction(SshActionManager* _manager, QString _local_dir, QString _remote_base_dir);
		void init();
		void execute();
		QString localDir() { return m_local_dir; }
		QString remoteBaseDir() { return m_remote_base_dir; }
		QString progressMessage() {
                  return QString("Uploading files to '%1'")
                      .arg(m_remote_base_dir);
                }

	protected:
		void doTerminate();

	private:
		QString m_local_dir;
		QString m_remote_base_dir;
	};

	class ListDirContentAction : public SshAction 
	{
    friend class SshActionManager;

  public:
    explicit ListDirContentAction(SshActionManager* _manager,
			QString _remote_dir, 
			FileTypeFilters _flags = FileTypeFilter::Dirs, 
			QStringList _file_filters = QStringList());
    void init();
    void execute();
    QString remoteDir() { return m_remote_dir; }
    QString progressMessage() {
      return QString("Listing contents for dir '%1'").arg(m_remote_dir);
    }

  protected:
    void doTerminate();

  private:
    QString m_remote_dir;
		FileTypeFilters m_flags;
    QStringList m_file_filters;
  };
	
	class SendCommandAction : public SshAction
	{
		friend class SshActionManager;

	public:
		explicit SendCommandAction(SshActionManager* _manager, QString _command_string);
		void init();
		void execute();
		QString command() { return m_command; }
		QString setResponse(QString _response) { m_response = _response; }
		QString response() { return m_response; }
		QString progressMessage() {
			return QString("Executing command '%1'").arg(m_command); }

	protected:
    void doTerminate();

  private:
		QString m_command;
		QString m_response;
	};


class DownloadDirAction : public SshAction {

 public:
  explicit DownloadDirAction(SshActionManager* _manager, QString _remote_dir,
                          QString _local_base_dir);
	void init();
	void execute();
	QString remoteDir() { return m_remote_dir; }
	QString localBaseDir() { return m_local_base_dir; }
	QString progressMessage() {
          return QString("Downloading files from '%1'").arg(m_remote_dir);
        }

 protected:
	void doTerminate();

 private:
	QString m_remote_dir;
	QString m_local_base_dir;
};

}

#endif  // MATISSE_SSH_ACTION_H_