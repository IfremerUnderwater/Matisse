#include "parameters_dialog.h"
#include "ui_parameters_dialog.h"

namespace matisse {

QString ParametersDialog::_filenameExp = "(\\w+\\s+)+";
QString ParametersDialog::_diacriticLetters = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
QStringList ParametersDialog::_noDiacriticLetters = QStringList() << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";

ParametersDialog::ParametersDialog(QWidget *parent, QString path, KeyValueList fields) :
   QDialog(parent),
   _ui(new Ui::ParametersDialog),
   _path(path)
{
    _ui->setupUi(this);
    init(fields);
}

ParametersDialog::~ParametersDialog()
{
    delete _ui;
}

QString ParametersDialog::removeAccents(QString str)
{
    QString output = "";
   for (int i = 0; i < str.length(); i++) {
       QChar c = str[i];
       int dIndex = _diacriticLetters.indexOf(c);
       if (dIndex < 0) {
           output.append(c);
       } else {
           QString replacement = _noDiacriticLetters[dIndex];
           output.append(replacement);
       }
   }

   return output;
}

QString ParametersDialog::getFilename()
{
    return _path + "/" + _filename;
}

KeyValueList ParametersDialog::getFields()
{
    KeyValueList keyValueList;

    keyValueList.set("modelVersion", _ui->_LE_modele->text());
    keyValueList.set("name", _ui->_LE_name->text());
    keyValueList.set("date", QDate::currentDate().toString("dd/MM/yyyy"));
    keyValueList.set("author", _ui->_LE_author->text());
    keyValueList.set("comments", _ui->_TXT_comments->toPlainText());

    return keyValueList;

}

void ParametersDialog::init( KeyValueList fields)
{
    _ui->_LE_modele->setText(fields.getValue("modelVersion"));
    _ui->_LE_author->setText(fields.getValue("author"));
    _ui->_LE_name->setText(fields.getValue("name"));
    slot_filename(_ui->_LE_name->text());
    _ui->_LE_name->setValidator(new QRegExpValidator(QRegExp(_filenameExp)));
    _ui->_TXT_comments->setPlainText(fields.getValue("comments"));

    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_save()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_ui->_LE_name, SIGNAL(textChanged(QString)), this, SLOT(slot_filename(QString)));

    _ui->_PB_save->setEnabled(false);
}

void ParametersDialog::slot_filename(QString name)
{
    name = name.trimmed();
    if (!name.isEmpty()) {
        name = name.replace(" ", "_") + ".xml";
        //foreach(name.a)
    }
    _ui->_LA_filename->setText(removeAccents(name));
    _ui->_PB_save->setEnabled(!name.isEmpty());
}

void ParametersDialog::slot_save()
{
    // verification existence fichier
    _filename = _ui->_LE_name->text().trimmed().replace(" ", "_") + ".xml";
    QFileInfo info(_path, _filename);
    QString unableToSaveMsg = tr("Cannot save");
    if (info.exists()) {
        if (info.isWritable()) {
            QString msg1 = tr("Saving confirmation");
            QString msg2 = tr("File %1 already exists.\nDo you want to replace it ?").arg(_filename);
            if (QMessageBox::No == QMessageBox::question(this, msg1, msg2, QMessageBox::Yes, QMessageBox::No)) {
                return;
            }
        } else {

            QString msg2 = tr("File %1 already exists and cannot be replaced !").arg(_filename);
            QMessageBox::warning(this, unableToSaveMsg, msg2);
            return;
        }
    } else if (!QFileInfo(_path,"").isWritable()) {
        QMessageBox::warning(this, unableToSaveMsg, tr("Cannot write in destination folder ! (permission ?)"));
        return;
    }
    accept();
}

} // namespace matisse
