#include "network_client.h"

namespace network_tools {

NetworkClient::NetworkClient()
    : m_action_queue(), m_current_action(NULL)
{

}

void NetworkClient::init()
{
    if (!m_cx_wrapper) {
        qCritical() << "NetworkClient: connection wrapper not set";
        return;
    }

    connect(m_cx_wrapper, SIGNAL(si_clearConnection()), SLOT(sl_onClearConnection()));
    connect(m_cx_wrapper, SIGNAL(si_connected()), SLOT(sl_onConnected()));

    // client specific initialization
    doInit();
}

void NetworkClient::addAction(NetworkAction *_action) {
    if (!_action->isValid()) {
        qCritical() << "NetworkClient: Action type " << _action->type()
                    << " cannot be performed, will be skipped";
        return;
    }

    m_action_queue.enqueue(_action);
    if (!m_current_action) {
        processAction();
    }
}

void NetworkClient::processAction() {
    if (!m_cx_wrapper->isConnected()) {
        if (!m_cx_wrapper->isWaitingForConnection()) {
            m_cx_wrapper->connectToRemoteHost();
        }
        return;
    }

    if (m_action_queue.isEmpty()) {
        qWarning() << "NetworkClient: network action queue empty, no action to process";
        return;
    }


    /* Nominal case : free previous action instance */
    if (m_current_action) {
        disconnectAction(m_current_action);
        delete m_current_action;
    }

    doInitBeforeAction();

    m_current_action = m_action_queue.dequeue();
    connectAction(m_current_action);

    qDebug() << "NetworkClient: Processing network action of type " << m_current_action->type();
    m_current_action->init();
}

void NetworkClient::checkActionPending() {
    if (m_action_queue.isEmpty()) {
        m_cx_wrapper->disconnectFromHost();
    } else {
        /* If actions are still pending, start next action */
        processAction();
    }
}

void NetworkClient::resume() {
    /* Try to reconnect after failed login */
    m_cx_wrapper->connectToRemoteHost();
}

void NetworkClient::sl_onClearConnection() {
    clearConnectionAndActionQueue();
}

void NetworkClient::clearConnectionAndActionQueue() {
    m_cx_wrapper->disableConnection();

    if (!m_action_queue.isEmpty()) {
        qCritical() << QString(
                           "NetworkAction: Disconnected while %1 actions are still "
                           "pending in action queue, clearing queue...")
                       .arg(m_action_queue.count());
        qDeleteAll(m_action_queue);
        m_action_queue.clear();
    }

    /* free last action instance */
    if (m_current_action) {
        disconnectAction(m_current_action);
        delete m_current_action;
        m_current_action = NULL;
    }

    m_cx_wrapper->freeConnection();
}

void NetworkClient::sl_onConnected() {
    if (!m_current_action) {
        processAction();
    }
}

void NetworkClient::clearActions() {
    clearConnectionAndActionQueue();
}


//bool NetworkClient::isConnected() { return m_connected; }

//void NetworkClient::setHost(QString _host) {
//    m_host = _host;
//}

//QString NetworkClient::host() { return m_host; }

//void NetworkClient::setCredentials(NetworkCredentials *_creds) {
//    m_creds = _creds;
//}

//QString NetworkClient::username() { return m_creds->username(); }

void NetworkClient::setConnectionWrapper(ConnectionWrapper *_cx_wrapper) {
    m_cx_wrapper = _cx_wrapper;
}

ConnectionWrapper *NetworkClient::connectionWrapper() {
    return m_cx_wrapper;
}

}  // namespace network_tools
