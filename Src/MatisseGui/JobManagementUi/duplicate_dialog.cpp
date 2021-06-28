#include "duplicate_dialog.h"
#include "ui_duplicate_dialog.h"
#include <QRegExpValidator>

namespace matisse {

DuplicateDialog::DuplicateDialog(QWidget *_parent, QString _original_name, QString &_new_name, bool _is_assembly, QStringList _existing_elements, QStringList _archived_jobs) :
    QDialog(_parent),
    m_ui(new Ui::DuplicateDialog),
    m_original_name(_original_name),
    m_new_name(&_new_name),
    m_is_assembly(_is_assembly),
    m_existing_element_names(_existing_elements),
    m_archived_jobs(_archived_jobs)
{
    m_ui->setupUi(this);

    m_ui->_LE_newName->setValidator(new QRegExpValidator(QRegExp("\\w+(\\s|\\w)+\\w")));

    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(sl_close()));
}

DuplicateDialog::~DuplicateDialog()
{
    delete m_ui;
}

void DuplicateDialog::sl_close()
{
    if (sender() == m_ui->_PB_cancel) {
        reject();
    } else {
        QString name = m_ui->_LE_newName->text().trimmed();
        name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Cannot copy..."), tr("No name given"));
            return;
        }

        if (name == m_original_name) {
            QMessageBox::warning(this, tr("Cannot copy..."), tr("Name has to be different than previous one"));
            return;
        }

        if (m_existing_element_names.contains(name)) {
            if (m_is_assembly) {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is already used.").arg(name));
            } else {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is already used.").arg(name));
            }

            return;
        }

        if (!m_is_assembly) {
            if (m_archived_jobs.contains(name)) {
                QMessageBox::warning(this, tr("Cannot copy..."), tr("Name '%1' is used by an archived task.").arg(name));
                return;
            }
        }

        /* validate new name */
        *m_new_name = name;

        accept();
    }
}

} // namespace matisse

