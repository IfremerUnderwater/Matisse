#ifndef NETWORK_TOOLS_NETWORK_CLIENT_H_
#define NETWORK_TOOLS_NETWORK_CLIENT_H_

#include "network_commons.h"
#include "network_credentials.h"
#include "network_action.h"
#include "network_connector.h"

#include <QQueue>

namespace network_tools {


class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient();
    
    void init();
    void clearActions();

    void resume();
    void addAction(NetworkAction *_action);
    
    void setConnector(NetworkConnector * _cx_wrapper);
    NetworkConnector *connector();

signals:


public slots:
    void sl_onClearConnection();
    void sl_onConnected();


protected:
    virtual void doInit() = 0;
    virtual void doInitBeforeAction() = 0;
    virtual void connectAction(NetworkAction *_action) = 0;
    virtual void disconnectAction(NetworkAction *_action) = 0;

    void processAction();
    void checkActionPending();
    void clearConnectionAndActionQueue();

    NetworkConnector *m_connector;
    QQueue<NetworkAction*> m_action_queue;
    NetworkAction *m_current_action;
};


} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CLIENT_H_
