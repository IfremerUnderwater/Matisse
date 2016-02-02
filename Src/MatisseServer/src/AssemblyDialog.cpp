#include "AssemblyDialog.h"
#include "ui_AssemblyDialog.h"

using namespace MatisseServer;

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
        bool isRealTime = QVariant(keyValue.getValue("RealTime")).toBool();
        if (isRealTime) {
            _ui->_RB_realTime->setChecked(true);
        } else {
            _ui->_RB_deferredTime->setChecked(true);
        }
        _ui->_LE_version->setText(keyValue.getValue("Version"));
        _ui->_CK_valid->setChecked(keyValue.getValue("Valid").toShort());
        _ui->_TXT_comments->setText(keyValue.getValue("Comments"));
    }

    if (isNewAssembly) {
        setWindowTitle(tr("Nouvelle chaine de traitement..."));
        //_ui->_WID_properties->hide();
//        _ui->_LA_author->hide();
//        _ui->_LE_author->hide();
//        _ui->_HSP_author->hide();
//        _ui->_LA_version->hide();
//        _ui->_LA_valid->hide();
//        _ui->_LA_comments->hide();
//        _ui->_TXT_comments->hide();
        _ui->_PB_save->setText((tr("Creer")));

    } else {
        setWindowTitle(tr("Enregistrer la chaine de traitement..."));

        _ui->_RB_deferredTime->setEnabled(false);
        _ui->_RB_realTime->setEnabled(false);

        if (!isFirstTimeSave) {
            _ui->_LE_name->setEnabled(false);
            _ui->_LE_author->setEnabled(false);
            _ui->_PB_save->setText((tr("Enregistrer")));
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
        inputName.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
        *_name = inputName; // normalisation du nom saisi
        _keyValue->set("Author", _ui->_LE_author->text().trimmed());
        bool isRealTime = _ui->_RB_realTime->isChecked();
        _keyValue->set("RealTime", QString("%1").arg(isRealTime));
        _keyValue->set("Version", _ui->_LE_version->text().trimmed());
        _keyValue->set("Valid", QString("%1").arg(_ui->_CK_valid->isChecked()));
        _keyValue->set("Comments", _ui->_TXT_comments->toPlainText().trimmed());
        _keyValue->set("Date", QDateTime::currentDateTime().toString("dd/MM/yyyy"));

        accept();
    }
}
