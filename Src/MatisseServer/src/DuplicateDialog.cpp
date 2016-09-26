#include "DuplicateDialog.h"
#include "ui_DuplicateDialog.h"

using namespace MatisseServer;

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
        name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Duplication impossible..."), tr("Un nom doit obligatoirement etre fourni pour la tache"));
            return;
        }

        if (name == _originalName) {
            QMessageBox::warning(this, tr("Duplication impossible..."), tr("Le nouveau nom doit obligatoirement etre different du nom de la tache dupliquee"));
            return;
        }

        if (_existingElementNames.contains(name)) {
            if (_isAssembly) {
                QMessageBox::warning(this, tr("Duplication impossible..."), tr("Le nom de chaine de traitement '%1' est deja utilise.").arg(name));
            } else {
                QMessageBox::warning(this, tr("Duplication impossible..."), tr("Le nom de tache '%1' est deja utilise.").arg(name));
            }

            return;
        }

        if (!_isAssembly) {
            if (_archivedJobs.contains(name)) {
                QMessageBox::warning(this, tr("Duplication impossible..."), tr("Le nom '%1' est deja utilise par une tache archivee.").arg(name));
                return;
            }
        }

        /* validate new name */
        *_newName = name;

        accept();
    }
}

