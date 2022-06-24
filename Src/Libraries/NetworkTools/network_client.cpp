#include "network_client.h"

namespace network_tools {

NetworkClient::NetworkClient()
    : m_action_queue(),
      m_current_action(NULL)
{

}

void NetworkClient::init()
{
    if (!m_connector) {
        qCritical() << "NetworkClient: connection wrapper not set";
        return;
    }

    connect(m_connector, SIGNAL(si_clearConnection()), SLOT(sl_onClearConnection()));
    connect(m_connector, SIGNAL(si_connected()), SLOT(sl_onConnected()));

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
    qDebug() << "NetworkClient: Processing new action...";

    if (!m_connector) {
        qCritical() << "NetworkClient: network connector is NULL";
        return;
    }

    if (!m_connector->isConnected()) {
        if (!m_connector->isWaitingForConnection()) {
            m_connector->connectToRemoteHost();
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
        m_connector->disconnectFromHost();
    } else {
        /* If actions are still pending, start next action */
        processAction();
    }
}

void NetworkClient::resume() {
    /* Try to reconnect after failed login */
    m_connector->connectToRemoteHost();
}

void NetworkClient::sl_onClearConnection() {
    clearConnectionAndActionQueue();
}

void NetworkClient::clearConnectionAndActionQueue() {
    qDebug() << QString("NetworkClient: clearing connection and action queue...");

    m_connector->disableConnection();

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

    m_connector->freeConnection();
}

void NetworkClient::sl_onConnected() {
    if (!m_current_action) {
        processAction();
    }
}

void NetworkClient::clearActions() {
    clearConnectionAndActionQueue();
}

void NetworkClient::setConnector(NetworkConnector *_cx_wrapper) {
    m_connector = _cx_wrapper;
}

NetworkConnector *NetworkClient::connector() {
    return m_connector;
}

}  // namespace network_tools
