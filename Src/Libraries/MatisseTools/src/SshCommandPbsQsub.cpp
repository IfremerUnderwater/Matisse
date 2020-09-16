#include "SshCommandPbsQsub.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QtDebug>

namespace MatisseTools {

SshCommandPbsQsub::SshCommandPbsQsub() :
	SshCommand("qsub", 1)
{
}

bool SshCommandPbsQsub::checkArguments() 
{
	// Nothing
  return true;
}

void SshCommandPbsQsub::checkExecuted() 
{
  bool found_echo = false;
  QRegularExpression qsub_id_exp(QString::fromUtf8("^(\\d+)\\.(.*)$"));
  QRegularExpression qsub_error_exp(QString::fromUtf8("^qsub:(.+)$"));

	for (int i = 0; i < m_output_lines.count(); i++)
	{
    QString line = m_output_lines.at(i);

		if (found_echo) 
		{
			// Looking for scheduled job pattern
			QRegularExpressionMatch match = qsub_id_exp.match(line);
			if (match.hasMatch()) 
			{
        // if last line, skip and wait for prompt
        if (i == (m_output_lines.count() - 1)) 
        {
          qDebug() << "Prompt has not been returned yet, waiting...";
          break;
        } 

        QString job_id_str = match.captured(1);
        QString pbs_node = match.captured(2);

				qDebug() << QString("Job scheduled with id %1 on server %2")
                        .arg(job_id_str)
                        .arg(pbs_node);
        
        m_job_id = job_id_str.toInt();
        m_node = pbs_node;
        
        m_is_executed = true;
        m_is_successfull = true;
        break;
      }

      /* Looking for command error pattern */
      match = qsub_error_exp.match(line);
      if (match.hasMatch()) 
      {
        // if last line, skip and wait for prompt
        if (i == (m_output_lines.count() - 1)) {
          qDebug() << "Prompt has not been returned yet, waiting...";
          break;
        }

        m_cmd_error_msg = match.captured(1);

        qCritical() << QString("Job scheduling failed with msg '%1'")
                           .arg(line);

        m_is_executed = true;
        m_is_successfull = false;
        break;
      }

    }

		if (line.contains(fullCommandString())) 
		{
      qDebug() << "Found command echo at line " << i;
			found_echo = true;
    }

  }
}

}  // namespace MatisseTools