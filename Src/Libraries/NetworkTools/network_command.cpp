#include "SshCommand.h"

#include <QStringList>
#include <QRegularExpression>
#include <QtDebug>

namespace MatisseTools {

SshCommand::SshCommand(QString _command, int _min_args, int _max_args) : 
  m_command(_command),
  m_arguments(), 
  m_output_stream(),
  m_error_stream(),
  m_min_args(_min_args),
  m_max_args(_max_args),
  m_full_command_string() 
{
  if (m_command.isEmpty()) 
  {
    qCritical() << "SSH command empty";
    return;
  }

  if (_min_args < 0)
  {
    qCritical() << QString("Nb of arguments for SSH command %1 shall be greater or equal to 0").arg(_min_args);
    return;  
  }  
    
  m_is_valid = true;
    
  if (_max_args < 0) 
  {
    _max_args = _min_args;
  }
}

QString SshCommand::fullCommandString() 
{
  if (!m_full_command_string.isEmpty()) 
  {
    // Command completion was already checked and full command string resolved;
    return m_full_command_string;
  }

  checkCompletion();

  if (!m_is_checked) 
  {
    qCritical() << "SSH command not complete, returning empty string";
    return QString();
  }

  m_full_command_string = m_command;

  for (int i = 0; i < m_arguments.count() ; i++) 
  {
    m_full_command_string.append(" ").append(m_arguments.at(i));
  }

  return m_full_command_string;
}

SshCommand& SshCommand::arg(QString _arg) 
{
  if (!m_is_valid)
  {
    qCritical() << "SSH command invalid, cannot add argument";
    return *this;  
  }

  int arg_count = m_arguments.count();

  if (arg_count == m_max_args) 
  {
    qWarning()
        << QString("Max number of arguments %1 reached, cannot add argument %2")
               .arg(m_max_args)
               .arg(_arg);
    return *this;

  }

  if (_arg.isEmpty()) {
    qCritical() << QString("Argument #%1 for command %2 is empty, ignored")
                        .arg(arg_count)
                        .arg(m_command);
    return *this;
  }
  
  m_arguments.append(_arg);

  return *this;
}

void SshCommand::appendOutputStream(QByteArray _output_stream) {
  qDebug() << _output_stream;

  if (!m_codec) {
    m_codec = QTextCodec::codecForUtfText(_output_stream);
    qDebug() << "Received stream with codec: %1" << m_codec->name();
  }

  m_output_stream.append(_output_stream);  

  if (m_is_executed) 
  {
    qDebug()
        << "Command interpreted as executed, received output will be ignored";
    return;
  }

  normalizeOutput();
  checkExecuted();
}

void SshCommand::appendErrorStream(QByteArray _error_stream) 
{
  qDebug() << _error_stream;

  if (!m_codec) 
  {
    m_codec = QTextCodec::codecForUtfText(_error_stream);
    qDebug() << "Received stream with codec: %1" << m_codec->name();
  }

  m_error_stream.append(_error_stream);
}

bool SshCommand::isExecuted() 
{ 
  return m_is_executed; 
}

bool SshCommand::isSuccessfull() { return m_is_successfull; }

QString SshCommand::output() 
{
  if (m_output_stream.isEmpty()) 
  {
    return QString();
  }

  if (!m_codec) 
  {
    qCritical() << "Unresolved output stream codec";
    return QString();
  }

  QString output = m_codec->toUnicode(m_output_stream);
  return output; 
}

QString SshCommand::error() 
{ 
  if (m_error_stream.isEmpty()) {
    return QString();
  }

  if (!m_codec) {
    qCritical() << "Unresolved error stream codec";
    return QString();
  }

  QString error = m_codec->toUnicode(m_error_stream);
  return error;
}

QString SshCommand::cmdErrorMessage() { return m_cmd_error_msg; }

void SshCommand::checkCompletion() 
{ 
  if (!checkArgCount()) 
  {
    return;
  }
  if (checkArguments()) 
  {
    m_is_checked = true;
  }
}

bool SshCommand::checkArgCount() 
{
  int nb_args = m_arguments.count();

  if (nb_args < m_min_args) {
    qCritical()
        << QString("%1 arguments provided for SSH command %2, %3 are expected")
               .arg(nb_args)
               .arg(m_command)
               .arg(m_min_args);

    return false;
  }

  return true;
}

void SshCommand::normalizeOutput() 
{
  m_output = m_codec->toUnicode(m_output_stream);

  /* Reset output line buffer */
  m_output_lines.clear();

  /* split lines and process line by line */
  QStringList lines = m_output.split("\r\n", QString::SkipEmptyParts);
  for (int i = 0; i < lines.size(); i++) 
  {
    QString line = lines.at(i);
  /* eliminate trailing spaces */
    QString tline = line.trimmed();

    QRegularExpression esc1(QString::fromUtf8("\u001B\[[0-9;]*m"));
    QRegularExpression esc2(QString::fromUtf8("\u000F"));

    tline.remove(esc1);
    tline.remove(esc2);

    if (tline.isEmpty()) 
    {
      //lines.removeAt(i);
    } 
    else 
    {
      m_output_lines.append(tline);
      qDebug() << "Normalized output line:\n" << tline;
      //lines.replace(i, tline);
    }
  }
}

void SshCommand::normalizeError() 
{
}

void SshCommand::checkExecuted() 
{
}



}
