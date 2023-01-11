#include "network_credentials.h"

namespace network_tools {

NetworkCredentials::NetworkCredentials(QString _username, QString _password) {
    if (_username.isNull() || _username.isEmpty()) {
        qCritical() << "NetworkCredentials: username null or empty";
    }

    m_username = _username;

    if (_password.isNull() || _password.isEmpty()) {
        qCritical() << "NetworkCredentials: password null or empty";
    }

    m_password = _password;
}

} // namespace network_tools

