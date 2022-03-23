#include "connection_wrapper.h"

namespace network_tools {

ConnectionWrapper::ConnectionWrapper() :
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

void ConnectionWrapper::reinitProgressIndicators(quint64 _transfer_size) {
    //    qDebug() << QString("SecureConnectionWrapper: reinit progress indicators:");
    //    qDebug() << QString("Transfer size: %1").arg(_transfer_size);
    m_current_transfer_size = _transfer_size;
    m_total_received_bytes = 0;
    m_progress_matrix.clear();
    m_last_signalled_progress = 0;
}

} // namespace network_tools
