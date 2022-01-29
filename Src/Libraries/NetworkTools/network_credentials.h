#ifndef NETWORK_CLIENT_NETWORK_CREDENTIALS_H_
#define NETWORK_CLIENT_NETWORK_CREDENTIALS_H_

#include <QObject>
#include <QtDebug>

namespace network_tools {

class NetworkCredentials
{
public:
    explicit NetworkCredentials(QString _username, QString _password);

    QString username() { return m_username; };
    QString password() { return m_password; };

private:
    QString m_username;
    QString m_password;
};


} // namespace network_tools

#endif // NETWORK_CLIENT_NETWORK_CREDENTIALS_H_
