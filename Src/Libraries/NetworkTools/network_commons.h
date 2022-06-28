#ifndef NETWORK_TOOLS_NETWORK_COMMONS_H_
#define NETWORK_TOOLS_NETWORK_COMMONS_H_

#include <QObject>
#include <QtDebug>
#include <QFlags>
#include <QDateTime>

namespace network_tools {


Q_NAMESPACE

/*!
  * \brief SSH specific errors
  */
enum class eConnectionError {
    /// No error has occured
    NO_ERROR_NC,
    /// There was a network socket error
    SOCKET_ERROR_NC,
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
    NO_ERROR_NC,
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

/*!
  * \brief File transfer protocol to be used with the remote file server
  */
enum class eFileTransferProtocol {
    FTP,
    SFTP
};

Q_ENUM_NS(eFileTransferProtocol)

/*!
  * \brief Shell protocol to be used with the remote file server
  */
enum class eShellProtocol {
    SSH
};

Q_ENUM_NS(eShellProtocol)

enum class eFileTypeFilter {
  Dirs = 0x001,
  Files = 0x002,
  AllEntries = Dirs | Files
};
Q_DECLARE_FLAGS(FileTypeFilters, eFileTypeFilter)
Q_DECLARE_OPERATORS_FOR_FLAGS(FileTypeFilters)


class NetworkFileInfo : public QObject
{
//    Q_OBJECT
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

} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_COMMONS_H_
