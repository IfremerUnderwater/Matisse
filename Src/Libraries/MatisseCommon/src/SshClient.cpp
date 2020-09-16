#include "SshClient.h"

namespace MatisseCommon {

SshClientCredentials::SshClientCredentials(QString username, QString password) {
  if (username.isNull() || username.isEmpty()) {
    qCritical() << "SSH username null or empty";
  }

  _username = username;

  if (password.isNull() || password.isEmpty()) {
    qCritical() << "SSH password null or empty";
  }

  _password = password;
}

SshActionManager::SshActionManager() {}

SshFileInfo::SshFileInfo(QString _name, bool _is_dir, quint64 _size, QDateTime _last_modified)
    : m_name(_name), m_is_dir(_is_dir), m_size(_size), m_last_modified(_last_modified) {}

SshClient::SshClient(QObject *parent)
    : SshActionManager(), m_action_queue(), m_current_action(NULL) {}

void SshClient::addAction(SshAction *action) {
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

void SshClient::setHost(QString host) { m_host = host; }

void SshClient::setCredentials(SshClientCredentials *creds) { m_creds = creds; }

}  // namespace MatisseCommon