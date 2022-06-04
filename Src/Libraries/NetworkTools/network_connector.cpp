#include "network_connector.h"

namespace network_tools {

NetworkConnector::NetworkConnector() :
    QObject(),
    m_host(),
    m_shell_command(),
    m_operation_remote_path(),
    m_operation_local_path(),
    m_progress_matrix(),
    m_file_filters(),
    m_dir_contents_buffer(),
    m_dir_contents_received(false)
{

}

bool NetworkConnector::isConnected() {
    return m_connected;
}

bool NetworkConnector::isWaitingForConnection() {
    return m_waiting_for_connection;
}

void NetworkConnector::setHost(QString _host) {
    qDebug() << QString("NetworkConnector: setting host '%1'").arg(_host);
    m_host = _host;
}

QString NetworkConnector::host() {
    return m_host;
}

void NetworkConnector::setCredentials(NetworkCredentials *_creds) {
    m_creds = _creds;
}

QString NetworkConnector::username() {
    return m_creds->username();
}

void NetworkConnector::reinitProgressIndicators(quint64 _transfer_size) {
    //    qDebug() << QString("NetworkConnector: reinit progress indicators:");
    //    qDebug() << QString("Transfer size: %1").arg(_transfer_size);
    m_current_transfer_size = _transfer_size;
    m_total_received_bytes = 0;
    m_progress_matrix.clear();
    m_last_signalled_progress = 0;
}

} // namespace network_tools
