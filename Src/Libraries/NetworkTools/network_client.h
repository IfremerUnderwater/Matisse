#ifndef NETWORK_TOOLS_NETWORK_CLIENT_H_
#define NETWORK_TOOLS_NETWORK_CLIENT_H_

#include <QDateTime>
#include <QFlags>
#include <QObject>
#include <QQueue>
#include <QtDebug>

#include "network_action.h"

namespace network_tools {

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

Q_NAMESPACE

/*!
  * \brief SSH specific errors
  */
enum class eConnectionError {
  /// No error has occured
  NO_ERROR,
  /// There was a network socket error
  SOCKET_ERROR,
  /// The connection timed out
  TIMEOUT_ERROR,
  /// There was an error communicating with the server
  PROTOCOL_ERROR,
  /// There was a problem with the remote host key
  HOST_KEY_ERROR,
  /// We failed to read or parse the key file used for authentication
  KEY_FILE_ERROR,
  /// We failed to authenticate
  AUTHENTICATION_ERROR,
  /// The server closed our connection
  CLOSED_BY_SERVER_ERROR,
  /// The ssh-agent used for authenticating failed somehow
  AGENT_ERROR,
  /// Something bad happened on the server
  INTERNAL_ERROR
};

Q_ENUM_NS(eConnectionError)

enum class eTransferError {
  NO_ERROR,
  END_OF_FILE,
  FILE_NOT_FOUND,
  PERMISSION_DENIED,
  GENERIC_FAILURE,
  BAD_MESSAGE,
  NO_CONNECTION,
  CONNECTION_LOST,
  UNSUPPORTED_OPERATION
};

Q_ENUM_NS(eTransferError)

class NetworkClient : public QObject
{
    Q_OBJECT

public:


  explicit NetworkClient();
    
  virtual void init() = 0;
  virtual void resume() = 0;
  virtual void resetConnection() = 0;
  virtual void clearActions() = 0;

  void addAction(NetworkAction *_action);
    
  bool isConnected();
  void setHost(QString _host);
  QString host();
  void setCredentials(NetworkClientCredentials* _creds);
  QString username();

signals:
  void si_connectionFailed(eConnectionError _err);
  void si_connectionClosed();
  void si_progressUpdate(int _progress);

protected:
  virtual void processAction() = 0;
  virtual void connectAction(NetworkAction *_action) = 0;
  virtual void disconnectAction(NetworkAction *_action) = 0;

  QString m_host;
  NetworkClientCredentials *m_creds;
  QQueue<NetworkAction*> m_action_queue;
  NetworkAction *m_current_action;
  bool m_connected = false;
  bool m_waiting_for_connection = false;
  eConnectionError m_current_cx_error = eConnectionError::NO_ERROR;
  int m_last_signalled_progress = 0;
};


} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CLIENT_H_
