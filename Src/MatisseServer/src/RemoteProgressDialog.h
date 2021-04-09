#ifndef MATISSE_REMOTE_PROGRESS_DIALOG_H_
#define MATISSE_REMOTE_PROGRESS_DIALOG_H_

#include <QDialog>

namespace Ui {
class RemoteProgressDialog;
}

namespace MatisseServer {

class RemoteProgressDialog : public QDialog {
  Q_OBJECT

 public:
  RemoteProgressDialog(QWidget *parent);
  void reinit();

public slots:
  void sl_onProgressUpdate(int _new_value);
  void sl_onMessageUpdate(QString _new_message);

 private:
  Ui::RemoteProgressDialog *_ui;
};

}  // namespace MatisseServer

#endif // MATISSE_REMOTE_PROGRESS_DIALOG_H_
