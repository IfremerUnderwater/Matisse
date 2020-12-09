#include "RemoteProgressDialog.h"
#include "ui_RemoteProgressDialog.h"

#include <QtDebug>

namespace MatisseServer
{

RemoteProgressDialog::RemoteProgressDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::RemoteProgressDialog) {
  
  _ui->setupUi(this);
  _ui->m_PB_transfer_progress->setValue(0);
  _ui->m_LA_transfer_message->setWordWrap(true);
}

void RemoteProgressDialog::reinit() 
{
  _ui->m_PB_transfer_progress->setValue(0);
  _ui->m_LA_transfer_message->setText("");
}

void RemoteProgressDialog::sl_onMessageUpdate(QString _new_message) {
  _ui->m_LA_transfer_message->setText(_new_message);
}

void RemoteProgressDialog::sl_onProgressUpdate(int _new_value) 
{
  if (_new_value < 0 || _new_value > 100) {
    qWarning() << "RemoteProgressDialog: value not a percentage : " << _new_value;
    return;
  }

  _ui->m_PB_transfer_progress->setValue(_new_value);
}

}  // namespace MatisseServer
