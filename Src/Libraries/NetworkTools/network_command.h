#ifndef MATISSE_NETWORK_COMMAND_H_
#define MATISSE_NETWORK_COMMAND_H_

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTextCodec>

namespace MatisseTools {

class NetworkCommand :
  public QObject
{
  Q_OBJECT

public:
  explicit NetworkCommand(QString _command, int _min_args, int _max_args = -1);

  QString fullCommandString();

  NetworkCommand & arg(QString _arg);
  void appendOutputStream(QByteArray _output_stream);
  void appendErrorStream(QByteArray _error_stream);
  bool isExecuted();
  bool isSuccessfull();

  QString output();
  QString error();
  QString cmdErrorMessage();

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
  bool m_is_successfull = false;

  int m_min_args;
  int m_max_args;

  QString m_cmd_error_msg; // command specific error message (received on standard output channel)

private:
  void checkCompletion();
  bool checkArgCount();
  void normalizeOutput();
  void normalizeError();
  
  QTextCodec *m_codec = NULL;
  QByteArray m_output_stream;
  QByteArray m_error_stream;

  QString m_output; // raw output string (contains ANSI esc codes)
  QString m_error; // raw error string received from shell standard error channel
};

}

#endif  // MATISSE_NETWORK_COMMAND_H_
