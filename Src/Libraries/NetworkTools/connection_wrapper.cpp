#include "connection_wrapper.h"

namespace network_tools {

ConnectionWrapper::ConnectionWrapper() : QObject()
{

}

bool ConnectionWrapper::isConnected() {
    return m_connected;
}

bool ConnectionWrapper::isWaitingForConnection() {
    return m_waiting_for_connection;
}

void ConnectionWrapper::setHost(QString _host) {
    m_host = _host;
}

QString ConnectionWrapper::host() {
    return m_host;
}

void ConnectionWrapper::setCredentials(NetworkCredentials *_creds) {
    m_creds = _creds;
}

QString ConnectionWrapper::username() {
    return m_creds->username();
}

} // namespace network_tools
