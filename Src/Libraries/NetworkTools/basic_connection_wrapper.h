#ifndef NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_
#define NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_

#include <QObject>

namespace network_tools {

class BasicConnectionWrapper : public ConnectionWrapper
{
    Q_OBJECT
public:
    explicit BasicConnectionWrapper();
};

} // namespace network_tools

#endif // NETWORK_CLIENT_BASIC_CONNECTION_WRAPPER_H_
