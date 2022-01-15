#include "network_client.h"

namespace network_tools {

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

NetworkClient::NetworkClient()
    : m_action_queue(), m_current_action(NULL) {}

void NetworkClient::addAction(NetworkAction *_action) {
    if (!_action->isValid()) {
        qCritical() << "Action type " << _action->type()
                    << " cannot be performed, will be skipped";
        return;
    }

    m_action_queue.enqueue(_action);
    if (!m_current_action) {
        processAction();
    }
}

bool NetworkClient::isConnected() { return m_connected; }

void NetworkClient::setHost(QString _host) {
    m_host = _host;
}

QString NetworkClient::host() { return m_host; }

void NetworkClient::setCredentials(NetworkClientCredentials *_creds) {
    m_creds = _creds;
}

QString NetworkClient::username() { return m_creds->username(); }

void NetworkClient::setConnectionWrapper(ConnectionWrapper *_cx_wrapper) {
    m_cx_wrapper = _cx_wrapper;
}

}  // namespace network_tools
