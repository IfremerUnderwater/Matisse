#ifndef MATISSE_REMOTE_PROGRESS_DIALOG_H_
#define MATISSE_REMOTE_PROGRESS_DIALOG_H_

#include <QDialog>

namespace Ui {
class RemoteProgressDialog;
}

namespace matisse {

class RemoteProgressDialog : public QDialog {
  Q_OBJECT

 public:
  RemoteProgressDialog(QWidget *_parent);
  void reinit();

public slots:
  void sl_onProgressUpdate(int _new_value);
  void sl_onMessageUpdate(QString _new_message);

 private:
  Ui::RemoteProgressDialog *m_ui;
};

}  // namespace matisse

#endif // MATISSE_REMOTE_PROGRESS_DIALOG_H_
