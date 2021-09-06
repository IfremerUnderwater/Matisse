#ifndef MATISSE_REMOTE_FILE_DIALOG_H_
#define MATISSE_REMOTE_FILE_DIALOG_H_

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QDialog>

#include "network_client.h"
#include "tree_model.h"


namespace Ui {
class RemoteFileDialog;
}

namespace matisse {

class RemoteFileTreeModelFactory : public QObject
{
  Q_OBJECT

 public:
  RemoteFileTreeModelFactory();
  TreeModel *createModel(QString _root_folder_name,
                         QList<network_tools::NetworkFileInfo *> _file_info);
};


class RemoteFileDialog : public QDialog {
  Q_OBJECT

 public:
  RemoteFileDialog(TreeModel *_model, QWidget *parent);
  QString selectedFile() { return m_selected_file; }

 protected slots:
  void sl_onAccepted();
  void sl_onRejected();
  void sl_onSelectionChanged(const QModelIndex &_current, const QModelIndex &_previous);

 private:
  Ui::RemoteFileDialog *m_ui;
  QString m_selected_file;
};

}  // namespace matisse

#endif // MATISSE_REMOTE_FILE_DIALOG_H_
