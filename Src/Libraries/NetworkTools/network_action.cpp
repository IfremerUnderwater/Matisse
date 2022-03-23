#include "network_action.h"

#include <QtDebug>
#include <QFileInfo>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace network_tools {

NetworkAction::NetworkAction(eNetworkActionType _type) {
  m_type = _type;
}

void NetworkAction::terminate()
{ 
  m_is_terminated = true;

  /* Command specific termination */
  doTerminate();
}

}  // namespace network_tools
