#include "duplicate_dialog.h"
#include "ui_duplicate_dialog.h"
#include <QRegExpValidator>

namespace matisse {

DuplicateDialog::DuplicateDialog(QWidget *parent, QString originalName, QString &newName, bool isAssembly, QStringList existingElements, QStringList archivedJobs) :
    QDialog(parent),
    _ui(new Ui::DuplicateDialog),
    _originalName(originalName),
    _newName(&newName),
    _isAssembly(isAssembly),
    _existingElementNames(existingElements),
    _archivedJobs(archivedJobs)
{
    _ui->setupUi(this);

    _ui->_LE_newName->setValidator(new QRegExpValidator(QRegExp("\\w+(\\s|\\w)+\\w")));

    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
}

DuplicateDialog::~DuplicateDialog()
{
    delete _ui;
}

void DuplicateDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        QString name = _ui->_LE_newName->text().trimmed();
        //name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
        name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Cannot copy..."), tr("No name given"));
            return;
        }

        if (name == _originalName) {
            QMessageBox::warning(this, tr("Cannot copy..."), tr("Name has to be different than previous one"));
            return;
        }

        if (_existingElementNames.contains(name)) {
            if (_isAssembly) {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is already used.").arg(name));
            } else {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is already used.").arg(name));
            }

            return;
        }

        if (!_isAssembly) {
            if (_archivedJobs.contains(name)) {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is used by an archived task.").arg(name));
                return;
            }
        }

        /* validate new name */
        *_newName = name;

        accept();
    }
}

} // namespace matisse

