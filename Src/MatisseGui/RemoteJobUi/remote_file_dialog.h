#ifndef MATISSE_REMOTE_FILE_DIALOG_H_
#define MATISSE_REMOTE_FILE_DIALOG_H_

#include <QAbstractButton>
#include <QAbstractItemModel>
#include <QDialog>
#include <QValidator>

#include "matisse_icon_factory.h"
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

class RemotePathValidator : public QValidator {
public:
    RemotePathValidator(QObject *_parent);
    ~RemotePathValidator();

    QValidator::State validate(QString &_input, int &_pos) const;
};


class RemoteFileDialog : public QDialog {
    Q_OBJECT

public:
    RemoteFileDialog(TreeModel *_model, QString _root_folder, MatisseIconFactory *_icon_factory, QWidget *parent);
    QString selectedFile() { return m_selected_file; }

protected slots:
    void sl_onAccepted();
    void sl_onRejected();
    void sl_onSelectionChanged(const QModelIndex &_current, const QModelIndex &_previous);
    void sl_onGoToSubDir(const QModelIndex &_index);
    void sl_onPathChanged();
    void sl_onRestoreDefaultPath();
    void sl_onGoToParentDir();

signals:
    void si_updatePath(QString _new_path);
    void si_restoreDefaultPath();
    void si_goToParentDir();

private:
    Ui::RemoteFileDialog *m_ui;
    QString m_root_folder;
    QString m_selected_file;
};

}  // namespace matisse

#endif // MATISSE_REMOTE_FILE_DIALOG_H_
