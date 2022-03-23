#include "remote_file_dialog.h"
#include "ui_remote_file_dialog.h"

#include "file_utils.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QtDebug>

using namespace system_tools;
using namespace network_tools;

namespace matisse {

RemoteFileDialog::RemoteFileDialog(TreeModel *_model, QString _root_folder, QWidget *_parent)
    : QDialog(_parent), m_ui(new Ui::RemoteFileDialog), m_root_folder(_root_folder), m_selected_file() {
    m_ui->setupUi(this);
    m_ui->m_le_path->setText(m_root_folder);
    m_ui->m_le_path->setValidator(new RemotePathValidator(this));
    m_ui->m_tv_file_tree->setModel(_model);
    m_ui->m_tv_file_tree->expandAll(); // expand tree
    // resize columns
    for (int i=0 ; i < _model->columnCount() ; i++) {
        m_ui->m_tv_file_tree->resizeColumnToContents(i);
    }

    QPushButton *ok_button = m_ui->m_bb_buttons->button(QDialogButtonBox::Ok);
    ok_button->setEnabled(false); // OK button is disabled by default

    connect(m_ui->m_bb_buttons, SIGNAL(accepted()), SLOT(sl_onAccepted()));
    connect(m_ui->m_bb_buttons, SIGNAL(rejected()), SLOT(sl_onRejected()));
    connect(m_ui->m_le_path, SIGNAL(returnPressed()), SLOT(sl_onPathChanged()));
    connect(m_ui->m_pb_default_path, SIGNAL(clicked()), SLOT(sl_onRestoreDefaultPath()));
    connect(m_ui->m_pb_dir_up, SIGNAL(clicked()), SLOT(sl_onGoToParentDir()));

    connect(m_ui->m_tv_file_tree,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(sl_onGoToSubDir(const QModelIndex&)));

    QItemSelectionModel *sel = m_ui->m_tv_file_tree->selectionModel();
    connect(sel,
            SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
            SLOT(sl_onSelectionChanged(const QModelIndex &, const QModelIndex &)));

    if (m_root_folder.length() > 1 && m_root_folder.endsWith("/")) {
        /* Remove root folder trailing slash */
        m_root_folder.chop(1);
    }
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
    #if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
      index = index.siblingAtColumn(0);
    #else
      index = index.sibling(index.row(), 0);
    #endif
  }


    QVariant item = m_ui->m_tv_file_tree->model()->data(index);
    m_selected_file = item.toString();

    accept();
}

void RemoteFileDialog::sl_onRejected() 
{
    qDebug() << "QRemoteFileDialog: selection cancelled by user";
    reject();
}

void RemoteFileDialog::sl_onSelectionChanged(const QModelIndex &_current,
                                             const QModelIndex &_previous)
{
    Q_UNUSED(_previous)

    QPushButton *ok_button = m_ui->m_bb_buttons->button(QDialogButtonBox::Ok);

    QModelIndex parent = _current.parent();
    if (!parent.isValid()) {
        // Parent is invisible root, selected item is the non-selectable root folder
        ok_button->setEnabled(false);
        return;
    }

    ok_button->setEnabled(true);  // OK button enabled on selection
}

void RemoteFileDialog::sl_onGoToSubDir(const QModelIndex &_index) {
    QModelIndex parent = _index.parent();
    if (!parent.isValid()) {
        // Parent is invisible root, selected item is the non-selectable root folder
        return;
    }

    QModelIndex target_type_index = _index.siblingAtColumn(1);

    QVariant item_type = m_ui->m_tv_file_tree->model()->data(target_type_index);
    QString type = item_type.toString();

    if (type == "File") {
        qDebug() << "RemoteFileDialog: Double clicked on item of type File, ignoring";
        return;
    }

    QModelIndex target_index = _index;

    /* If folder was clicked from columns other than name */
    if (_index.column() > 0) {
        target_index = _index.siblingAtColumn(0);
    }

    QVariant item = m_ui->m_tv_file_tree->model()->data(target_index);
    QString sub_dir_name = item.toString();

    QString sub_dir_path = (m_root_folder == "/") ?
                m_root_folder + sub_dir_name :
                m_root_folder + "/" + sub_dir_name;
    emit si_updatePath(sub_dir_path);

    reject();
}

void RemoteFileDialog::sl_onPathChanged() {
    QString new_path = m_ui->m_le_path->text();

    if (new_path == m_root_folder) {
        return;
    }

    if (new_path.isEmpty()) {
        // restore initial path
        m_ui->m_le_path->setText(m_root_folder);
        return;
    }

    // TODO check path syntax

    emit si_updatePath(new_path);

    reject();
}

void RemoteFileDialog::sl_onRestoreDefaultPath()
{
    emit si_restoreDefaultPath();

    reject();
}

void RemoteFileDialog::sl_onGoToParentDir()
{
    emit si_goToParentDir();

    reject();
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

RemotePathValidator::RemotePathValidator(QObject *_parent) :
    QValidator(_parent)
{

}

RemotePathValidator::~RemotePathValidator()
{

}

QValidator::State RemotePathValidator::validate(QString &_input, int &_pos) const {
    Q_UNUSED(_pos)

    if (!FileUtils::isValidUnixPath(_input, true)) {
        qWarning() << QString("RemoteFileDialog: invalid path input by user '%1'").arg(_input);
        return QValidator::State::Invalid;
    }

    return QValidator::State::Acceptable;
}



}  // namespace matisse
