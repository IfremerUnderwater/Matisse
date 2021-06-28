#include "assembly_dialog.h"
#include "ui_assembly_dialog.h"

namespace matisse {

AssemblyDialog::AssemblyDialog(QWidget *_parent, QString &_name, KeyValueList &_key_value, bool _is_new_assembly, bool _is_first_time_save) :
    QDialog(_parent),
    m_ui(new Ui::AssemblyDialog)
{
    m_ui->setupUi(this);
    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(sl_close()));

    m_name = &_name;
    m_key_value = &_key_value;

    m_ui->_LE_name->setText(_name);

    if (m_key_value) {
        m_ui->_LE_author->setText(_key_value.getValue("Author"));
        m_ui->_LE_version->setText(_key_value.getValue("Version"));
        m_ui->_CK_valid->setChecked(_key_value.getValue("Valid").toShort());
        m_ui->_TXT_comments->setText(_key_value.getValue("Comments"));
    }

    if (_is_new_assembly) {
        setWindowTitle(tr("New processing chain..."));
        m_ui->_PB_save->setText((tr("Create")));

    } else {
        setWindowTitle(tr("Save processing chain..."));

        if (!_is_first_time_save) {
            m_ui->_LE_name->setEnabled(false);
            m_ui->_LE_author->setEnabled(false);
            m_ui->_PB_save->setText((tr("Save")));
        }
    }
}

AssemblyDialog::~AssemblyDialog()
{
    delete m_ui;
}

void AssemblyDialog::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

void AssemblyDialog::sl_close()
{
    if (sender() == m_ui->_PB_cancel) {
        reject();
    } else {
        QString input_name = m_ui->_LE_name->text();
        input_name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));
        *m_name = input_name; // normalisation du nom saisi
        m_key_value->set("Author", m_ui->_LE_author->text().trimmed());
        m_key_value->set("Version", m_ui->_LE_version->text().trimmed());
        m_key_value->set("Valid", QString("%1").arg(m_ui->_CK_valid->isChecked()));
        m_key_value->set("Comments", m_ui->_TXT_comments->toPlainText().trimmed());
        m_key_value->set("Date", QDateTime::currentDateTime().toString("dd/MM/yyyy"));

        accept();
    }
}

} // namespace matisse
