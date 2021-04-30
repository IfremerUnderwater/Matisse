#ifndef MATISSE_NETWORK_CLIENT_H_
#define MATISSE_NETWORK_CLIENT_H_

#include <QDateTime>
#include <QFlags>
#include <QObject>
#include <QQueue>
#include <QtDebug>

#include "network_action.h"

namespace MatisseCommon {

class NetworkClientCredentials
{
public:
    explicit NetworkClientCredentials(QString _username, QString _password);

    QString username() { return m_username; };
    QString password() { return m_password; };

private:
    QString m_username;
    QString m_password;
};


class NetworkFileInfo : public QObject
{
public:
  explicit NetworkFileInfo(QString _name, bool _is_dir, quint64 _size, QDateTime _last_modified);

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


class NetworkClient : public NetworkActionManager
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

  explicit NetworkClient(QObject *parent = nullptr);
    
  virtual void init() = 0;
  virtual void resume() = 0;
  virtual void resetConnection() = 0;
  virtual void clearActions() = 0;

  void addAction(NetworkAction *action);
    
  bool isConnected();
  void setHost(QString host);
  QString host();
  void setCredentials(NetworkClientCredentials* creds);
  QString username();

signals:
  void si_transferFinished(NetworkAction *_action);
  void si_transferFailed(NetworkAction *_action, NetworkClient::TransferError _err);
  void si_dirContents(QList<NetworkFileInfo *> _contents);
  void si_connectionFailed(NetworkClient::ConnectionError err);
  void si_connectionClosed();
  void si_shellOutputReceived(NetworkAction *action, QByteArray output);
  void si_shellErrorReceived(NetworkAction *action, QByteArray error);
  void si_progressUpdate(int _progress);

public slots:

protected:
  virtual void processAction() = 0;
    
  QString m_host;
  NetworkClientCredentials *m_creds;
  QQueue<NetworkAction*> m_action_queue;
  NetworkAction *m_current_action;
  bool m_connected = false;
  bool m_waiting_for_connection = false;
  ConnectionError m_current_cx_error = ConnectionError::NoError;
  TransferError m_current_tx_error = TransferError::NoError;
};


}

#endif // MATISSE_NETWORK_CLIENT_H_
