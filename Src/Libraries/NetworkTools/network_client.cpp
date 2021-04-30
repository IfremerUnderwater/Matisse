#include "network_client.h"

namespace MatisseCommon {

NetworkClientCredentials::NetworkClientCredentials(QString _username, QString _password) {
  if (_username.isNull() || _username.isEmpty()) {
    qCritical() << "SSH username null or empty";
  }

  m_username = _username;

  if (_password.isNull() || _password.isEmpty()) {
    qCritical() << "SSH password null or empty";
  }

  m_password = _password;
}

NetworkFileInfo::NetworkFileInfo(QString _name, bool _is_dir, quint64 _size, QDateTime _last_modified)
    : m_name(_name), m_is_dir(_is_dir), m_size(_size), m_last_modified(_last_modified) {}

NetworkClient::NetworkClient(QObject *parent)
    : NetworkActionManager(), m_action_queue(), m_current_action(NULL) {}

void NetworkClient::addAction(NetworkAction *action) {
  if (!action->isValid()) {
    qCritical() << "Action type " << action->type()
                << " cannot be performed, will be skipped";
    return;
  }

  m_action_queue.enqueue(action);
  if (!m_current_action) {
    processAction();
  }
}

bool NetworkClient::isConnected() { return m_connected; }

void NetworkClient::setHost(QString host) {
  m_host = host; 
}

QString NetworkClient::host() { return m_host; }

void NetworkClient::setCredentials(NetworkClientCredentials *creds) {
  m_creds = creds; 
}

QString NetworkClient::username() { return m_creds->username(); }

}  // namespace MatisseCommon
