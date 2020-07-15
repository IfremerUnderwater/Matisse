#ifndef MATISSE_SSH_COMMAND_PBS_QSUB_H_
#define MATISSE_SSH_COMMAND_PBS_QSUB_H_

#include "SshCommand.h"

namespace MatisseTools {

class SshCommandPbsQsub : public SshCommand 
{
public:
  explicit SshCommandPbsQsub();

protected:
  bool checkArguments();
  void checkExecuted();
};

}  // namespace MatisseTools

#endif // MATISSE_SSH_COMMAND_PBS_QSUB_H_
