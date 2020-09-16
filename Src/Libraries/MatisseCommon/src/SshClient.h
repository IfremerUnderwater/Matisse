#ifndef MATISSE_SSH_CLIENT_H_
#define MATISSE_SSH_CLIENT_H_

#include <QDateTime>
#include <QFlags>
#include <QObject>
#include <QQueue>
#include <QtDebug>

//#include "SshActionManager.h"
#include "SshAction.h"

namespace MatisseCommon {

class SshClientCredentials
{
public:
    explicit SshClientCredentials(QString _username, QString _password);

    QString username() { return _username; };
    QString password() { return _password; };

private:
    QString _username;
    QString _password;
};


class SshFileInfo : public QObject
{
public:
  explicit SshFileInfo(QString _name, bool _is_dir, quint64 _size, QDateTime _last_modified);

  QString name() { return m_name; }
  bool isDir() { return m_is_dir; }
  quint64 size() { return m_size; }
  QDateTime lastModified() { return m_last_modified; }

private:
  QString m_name;
  bool m_is_dir;
  quint64 m_size;
  QDateTime m_last_modified;
};


class SshClient : public SshActionManager
{
    Q_OBJECT

public:
    /*!
     * \brief SSH specific errors
     */
    enum class ConnectionError {
        /// No error has occured
        NoError,
        /// There was a network socket error
        SocketError,
        /// The connection timed out
        TimeoutError,
        /// There was an error communicating with the server
        ProtocolError,
        /// There was a problem with the remote host key
        HostKeyError,
        /// We failed to read or parse the key file used for authentication
        KeyFileError,
        /// We failed to authenticate
        AuthenticationError,
        /// The server closed our connection
        ClosedByServerError,
        /// The ssh-agent used for authenticating failed somehow
        AgentError,
        /// Something bad happened on the server
        InternalError
    };
    
    enum class TransferError {
      NoError,
      EndOfFile,
      FileNotFound,
      PermissionDenied,
      GenericFailure,
      BadMessage,
      NoConnection,
      ConnectionLost,
      UnsupportedOperation
    };

    Q_ENUM(ConnectionError)
    Q_ENUM(TransferError)

    explicit SshClient(QObject *parent = nullptr);
    virtual void connectToHost() = 0;
    virtual void disconnectFromHost() = 0;

    void addAction(SshAction *action);
    virtual void init() = 0;
    virtual void resume() = 0;

    void setHost(QString host);
    void setCredentials(SshClientCredentials* creds);

signals:
  void si_transferFinished(SshAction *_action);
 void si_transferFailed(SshAction *_action, SshClient::TransferError _err);
  void si_dirContents(QList<SshFileInfo *> _contents);
  void si_connectionFailed(SshClient::ConnectionError err);
  void si_shellOutputReceived(SshAction *action, QByteArray output);
  void si_shellErrorReceived(SshAction *action, QByteArray error);
  void si_progressUpdate(int _progress);

public slots:

protected:
    virtual void processAction() = 0;
    
    QString m_host;
    SshClientCredentials *m_creds;
    QQueue<SshAction*> m_action_queue;
    SshAction *m_current_action;
    bool m_connected = false;
    bool m_waiting_for_connection = false;
    ConnectionError m_current_cx_error = ConnectionError::NoError;
    TransferError m_current_tx_error = TransferError::NoError;
};


}

#endif // MATISSE_SSH_CLIENT_H_
