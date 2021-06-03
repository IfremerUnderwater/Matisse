#include "assembly_dialog.h"
#include "ui_assembly_dialog.h"

namespace matisse {

AssemblyDialog::AssemblyDialog(QWidget *parent, QString &name, KeyValueList &keyValue, bool isNewAssembly, bool isFirstTimeSave) :
    QDialog(parent),
    _ui(new Ui::AssemblyDialog)
{
    _ui->setupUi(this);
    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));

    _name = &name;
    _keyValue = &keyValue;

    _ui->_LE_name->setText(name);

    if (_keyValue) {
        _ui->_LE_author->setText(keyValue.getValue("Author"));
        _ui->_LE_version->setText(keyValue.getValue("Version"));
        _ui->_CK_valid->setChecked(keyValue.getValue("Valid").toShort());
        _ui->_TXT_comments->setText(keyValue.getValue("Comments"));
    }

    if (isNewAssembly) {
        setWindowTitle(tr("New processing chain..."));
        //_ui->_WID_properties->hide();
        //        _ui->_LA_author->hide();
        //        _ui->_LE_author->hide();
        //        _ui->_HSP_author->hide();
        //        _ui->_LA_version->hide();
        //        _ui->_LA_valid->hide();
        //        _ui->_LA_comments->hide();
        //        _ui->_TXT_comments->hide();
        _ui->_PB_save->setText((tr("Create")));

    } else {
        setWindowTitle(tr("Save processing chain..."));

        if (!isFirstTimeSave) {
            _ui->_LE_name->setEnabled(false);
            _ui->_LE_author->setEnabled(false);
            _ui->_PB_save->setText((tr("Save")));
        }
    }
}

AssemblyDialog::~AssemblyDialog()
{
    delete _ui;
}

void AssemblyDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
}

void AssemblyDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        QString inputName = _ui->_LE_name->text();
        //inputName.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
        inputName.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));
        *_name = inputName; // normalisation du nom saisi
        _keyValue->set("Author", _ui->_LE_author->text().trimmed());
        _keyValue->set("Version", _ui->_LE_version->text().trimmed());
        _keyValue->set("Valid", QString("%1").arg(_ui->_CK_valid->isChecked()));
        _keyValue->set("Comments", _ui->_TXT_comments->toPlainText().trimmed());
        _keyValue->set("Date", QDateTime::currentDateTime().toString("dd/MM/yyyy"));

        accept();
    }
}

} // namespace matisse
