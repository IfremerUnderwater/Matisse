#ifndef MATISSE_SSH_COMMAND_H_
#define MATISSE_SSH_COMMAND_H_

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTextCodec>

namespace MatisseTools {

class SshCommand :
  public QObject
{
  Q_OBJECT

public:
  explicit SshCommand(QString _command, int _min_args, int _max_args = -1);

  QString fullCommandString();

  SshCommand & arg(QString _arg);
  void appendOutputStream(QByteArray _output_stream);
  void appendErrorStream(QByteArray _error_stream);
  bool isExecuted();

  QString output();
  QString error();

protected:

  virtual bool checkArguments() = 0;
  virtual void checkExecuted() = 0;

  QString m_command; ///< Command name without arguments
  QList<QString> m_arguments;
  QString m_full_command_string; ///< Command string with arguments;
  QList<QString> m_output_lines; ///< Normalized shell output lines;

  bool m_is_valid = false;
  bool m_is_checked = false;
  bool m_is_executed = false;

  int m_min_args;
  int m_max_args;

private:
  void checkCompletion();
  bool checkArgCount();
  void normalizeOutput();
  void normalizeError();
  
  QTextCodec *m_codec = NULL;
  QByteArray m_output_stream;
  QByteArray m_error_stream;

  QString m_output; // raw output string (contains ANSI esc codes)
  QString m_error;
};

}

#endif  // MATISSE_SSH_COMMAND_H_