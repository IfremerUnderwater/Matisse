#include "remote_progress_dialog.h"
#include "ui_remote_progress_dialog.h"

#include <QtDebug>

namespace matisse
{

RemoteProgressDialog::RemoteProgressDialog(QWidget *_parent)
    : QDialog(_parent), m_ui(new Ui::RemoteProgressDialog) {
  
  m_ui->setupUi(this);
  m_ui->m_PB_transfer_progress->setValue(0);
  m_ui->m_LA_transfer_message->setWordWrap(true);
}

void RemoteProgressDialog::reinit() 
{
  m_ui->m_PB_transfer_progress->setValue(0);
  m_ui->m_LA_transfer_message->setText("");
}

void RemoteProgressDialog::sl_onMessageUpdate(QString _new_message) {
  m_ui->m_LA_transfer_message->setText(_new_message);
}

void RemoteProgressDialog::sl_onProgressUpdate(int _new_value) 
{
  if (_new_value < 0 || _new_value > 100) {
    qWarning() << "RemoteProgressDialog: value not a percentage : " << _new_value;
    return;
  }

  m_ui->m_PB_transfer_progress->setValue(_new_value);
}

}  // namespace matisse
