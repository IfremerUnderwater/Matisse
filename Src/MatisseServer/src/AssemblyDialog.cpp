#include "AssemblyDialog.h"
#include "ui_AssemblyDialog.h"

using namespace MatisseServer;

AssemblyDialog::AssemblyDialog(QWidget *parent, QString &name, KeyValueList &keyValue) :
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
        _ui->_CK_valid->setChecked(keyValue.getValue("valid").toShort());
        _ui->_TXT_comments->setText(keyValue.getValue("Comments"));
    }
}

AssemblyDialog::~AssemblyDialog()
{
    delete _ui;
}

void AssemblyDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        *_name = _ui->_LE_name->text();
        _keyValue->set("Author", _ui->_LE_author->text().trimmed());
        _keyValue->set("Version", _ui->_LE_version->text().trimmed());
        _keyValue->set("valid", QString("%1").arg(_ui->_CK_valid->isChecked()));
        _keyValue->set("Comments", _ui->_TXT_comments->toPlainText().trimmed());
        _keyValue->set("Date", QDateTime::currentDateTime().toString("dd/MM/yyyy"));

        accept();
    }
}
