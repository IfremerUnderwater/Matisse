#ifndef MATISSE_SSH_COMMAND_PBS_QSUB_H_
#define MATISSE_SSH_COMMAND_PBS_QSUB_H_

#include "network_command.h"

namespace MatisseTools {

class SshCommandPbsQsub : public NetworkCommand 
{
  Q_OBJECT

public:
  explicit SshCommandPbsQsub();

  int jobId() { return m_job_id; }
  QString node() { return m_node; }

protected:
  bool checkArguments();
  void checkExecuted();

private:
  int m_job_id = -1;
  QString m_node;
};

}  // namespace MatisseTools

#endif // MATISSE_SSH_COMMAND_PBS_QSUB_H_
