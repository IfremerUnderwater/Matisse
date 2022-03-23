#ifndef NETWORK_TOOLS_NETWORK_CLIENT_H_
#define NETWORK_TOOLS_NETWORK_CLIENT_H_

#include "network_commons.h"
#include "network_credentials.h"
#include "network_action.h"
#include "connection_wrapper.h"

#include <QQueue>

namespace network_tools {


class NetworkClient : public QObject
{
    Q_OBJECT

public:
    explicit NetworkClient();
    
    void init();
//    virtual void resetConnection() = 0;
    void clearActions();

    void resume();
    void addAction(NetworkAction *_action);
    
//    bool isConnected();
//    void setHost(QString _host);
//    QString host();
//    void setCredentials(NetworkCredentials* _creds);
//    QString username();
    void setConnectionWrapper(ConnectionWrapper * _cx_wrapper);
    ConnectionWrapper *connectionWrapper();

signals:
//    void si_connectionFailed(eConnectionError _err);
//    void si_connectionClosed();
//    void si_progressUpdate(int _progress);

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

//    QString m_host;
//    NetworkCredentials *m_creds;
    ConnectionWrapper *m_cx_wrapper;
    QQueue<NetworkAction*> m_action_queue;
    NetworkAction *m_current_action;
//    bool m_connected = false;
//    bool m_waiting_for_connection = false;
//    eConnectionError m_current_cx_error = eConnectionError::NO_ERROR;
//    int m_last_signalled_progress = 0;
};


} // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_CLIENT_H_
