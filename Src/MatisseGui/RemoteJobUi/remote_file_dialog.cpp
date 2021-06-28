#include "remote_file_dialog.h"
#include "ui_remote_file_dialog.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QtDebug>

using namespace network_tools;

namespace matisse {

RemoteFileDialog::RemoteFileDialog(TreeModel *_model, QWidget *_parent)
    : QDialog(_parent), m_ui(new Ui::RemoteFileDialog), m_selected_file() {
  m_ui->setupUi(this);
  m_ui->m_tv_file_tree->setModel(_model);
  m_ui->m_tv_file_tree->expandAll();
  QPushButton *ok_button = m_ui->m_bb_buttons->button(QDialogButtonBox::Ok);
  ok_button->setEnabled(false); // OK button is disabled by default

  connect(m_ui->m_bb_buttons, SIGNAL(accepted()), SLOT(sl_onAccepted()));
  connect(m_ui->m_bb_buttons, SIGNAL(rejected()), SLOT(sl_onRejected()));

  QItemSelectionModel *sel = m_ui->m_tv_file_tree->selectionModel();
  connect(sel,
          SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
          SLOT(sl_onSelectionChanged(const QModelIndex &, const QModelIndex &)));
}

void RemoteFileDialog::sl_onAccepted() 
{
  qDebug() << "QRemoteFileDialog: accepted";
  QItemSelectionModel *sel = m_ui->m_tv_file_tree->selectionModel();
  QModelIndex index = sel->currentIndex();

  if (!index.isValid()) {
    qCritical() << "QRmoteFileDialog: invalid selection";
    reject();  
  }

  /* If file was selected from columns other than name */
  if (index.column() > 0) {
    index = index.siblingAtColumn(0);
  }

  QVariant item = m_ui->m_tv_file_tree->model()->data(index);
  m_selected_file = item.toString();

  accept();
}

void RemoteFileDialog::sl_onRejected() 
{
  qDebug() << "QRemoteFileDialog: rejected";
  reject();
}

void RemoteFileDialog::sl_onSelectionChanged(const QModelIndex &_current,
                                            const QModelIndex &_previous) 
{
  QPushButton *ok_button = m_ui->m_bb_buttons->button(QDialogButtonBox::Ok);

  QModelIndex parent = _current.parent();
  if (!parent.isValid()) {
    // Parent is invisible root, selected item is the non-selectable root folder
    ok_button->setEnabled(false);
    return;
  }

  ok_button->setEnabled(true);  // OK button enabled on selection
}


RemoteFileTreeModelFactory::RemoteFileTreeModelFactory() {}

TreeModel *RemoteFileTreeModelFactory::createModel(QString _root_folder_name, QList<NetworkFileInfo *> _file_info) 
{
  QString folder_label = tr("Folder");
  QString file_label = tr("File");

  QString data_as_string;
  data_as_string.append(_root_folder_name)
      .append("\t").append(folder_label)
      .append("\t").append("-")
      .append("\t").append("-")
      .append("\n");

  QString indent = " ";

  /* Format data as a multiline string */
  for (NetworkFileInfo *sfi : _file_info) {
    QString file_type_string = (sfi->isDir()) ? folder_label : file_label;
    QString size_str = QLocale::system().toString(sfi->size());
    QString timestamp = sfi->lastModified().toString("dd/MM/yyyy HH:mm:ss");

    QString line;
    line.append(indent).append(sfi->name())
      .append("\t").append(file_type_string)
      .append("\t").append(size_str)
      .append("\t").append(timestamp);
    data_as_string.append(line).append("\n");
  }

  QStringList headers;
  headers << tr("Name") << tr("Type") << tr("Size") << tr("Last modification");

  return new TreeModel(data_as_string, headers);
}

}  // namespace matisse
