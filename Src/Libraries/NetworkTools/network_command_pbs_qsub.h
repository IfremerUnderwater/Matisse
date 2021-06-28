#ifndef NETWORK_TOOLS_NETWORK_COMMAND_PBS_QSUB_H_
#define NETWORK_TOOLS_NETWORK_COMMAND_PBS_QSUB_H_

#include "network_command.h"

namespace network_tools {

class NetworkCommandPbsQsub : public NetworkCommand
{
  Q_OBJECT

public:
  explicit NetworkCommandPbsQsub();

  int jobId() { return m_job_id; }
  QString node() { return m_node; }

protected:
  bool checkArguments();
  void checkExecuted();

private:
  int m_job_id = -1;
  QString m_node;
};

}  // namespace network_tools

#endif // NETWORK_TOOLS_NETWORK_COMMAND_PBS_QSUB_H_
