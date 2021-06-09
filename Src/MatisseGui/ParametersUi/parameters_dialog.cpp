#include "parameters_dialog.h"
#include "ui_parameters_dialog.h"

namespace matisse {

QString ParametersDialog::m_filename_exp = "(\\w+\\s+)+";
QString ParametersDialog::m_diacritic_letters = QString::fromUtf8("ŠŒŽšœžŸ¥µÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýÿ");
QStringList ParametersDialog::m_no_diacritic_letters = QStringList() << "S"<<"OE"<<"Z"<<"s"<<"oe"<<"z"<<"Y"<<"Y"<<"u"<<"A"<<"A"<<"A"<<"A"<<"A"<<"A"<<"AE"<<"C"<<"E"<<"E"<<"E"<<"E"<<"I"<<"I"<<"I"<<"I"<<"D"<<"N"<<"O"<<"O"<<"O"<<"O"<<"O"<<"O"<<"U"<<"U"<<"U"<<"U"<<"Y"<<"s"<<"a"<<"a"<<"a"<<"a"<<"a"<<"a"<<"ae"<<"c"<<"e"<<"e"<<"e"<<"e"<<"i"<<"i"<<"i"<<"i"<<"o"<<"n"<<"o"<<"o"<<"o"<<"o"<<"o"<<"o"<<"u"<<"u"<<"u"<<"u"<<"y"<<"y";

ParametersDialog::ParametersDialog(QWidget *_parent, QString _path, KeyValueList _fields) :
   QDialog(_parent),
   m_ui(new Ui::ParametersDialog),
   m_path(_path)
{
    m_ui->setupUi(this);
    init(_fields);
}

ParametersDialog::~ParametersDialog()
{
    delete m_ui;
}

QString ParametersDialog::removeAccents(QString _str)
{
    QString output = "";
   for (int i = 0; i < _str.length(); i++) {
       QChar c = _str[i];
       int d_index = m_diacritic_letters.indexOf(c);
       if (d_index < 0) {
           output.append(c);
       } else {
           QString replacement = m_no_diacritic_letters[d_index];
           output.append(replacement);
       }
   }

   return output;
}

QString ParametersDialog::getFilename()
{
    return m_path + "/" + m_filename;
}

KeyValueList ParametersDialog::getFields()
{
    KeyValueList kvl;

    kvl.set("modelVersion", m_ui->_LE_modele->text());
    kvl.set("name", m_ui->_LE_name->text());
    kvl.set("date", QDate::currentDate().toString("dd/MM/yyyy"));
    kvl.set("author", m_ui->_LE_author->text());
    kvl.set("comments", m_ui->_TXT_comments->toPlainText());

    return kvl;

}

void ParametersDialog::init(KeyValueList _fields)
{
    m_ui->_LE_modele->setText(_fields.getValue("modelVersion"));
    m_ui->_LE_author->setText(_fields.getValue("author"));
    m_ui->_LE_name->setText(_fields.getValue("name"));
    sl_filename(m_ui->_LE_name->text());
    m_ui->_LE_name->setValidator(new QRegExpValidator(QRegExp(m_filename_exp)));
    m_ui->_TXT_comments->setPlainText(_fields.getValue("comments"));

    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_save()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(m_ui->_LE_name, SIGNAL(textChanged(QString)), this, SLOT(sl_filename(QString)));

    m_ui->_PB_save->setEnabled(false);
}

void ParametersDialog::sl_filename(QString _name)
{
    _name = _name.trimmed();
    if (!_name.isEmpty()) {
        _name = _name.replace(" ", "_") + ".xml";
        //foreach(name.a)
    }
    m_ui->_LA_filename->setText(removeAccents(_name));
    m_ui->_PB_save->setEnabled(!_name.isEmpty());
}

void ParametersDialog::sl_save()
{
    // verification existence fichier
    m_filename = m_ui->_LE_name->text().trimmed().replace(" ", "_") + ".xml";
    QFileInfo info(m_path, m_filename);
    QString unable_to_save_msg = tr("Cannot save");
    if (info.exists()) {
        if (info.isWritable()) {
            QString msg1 = tr("Saving confirmation");
            QString msg2 = tr("File %1 already exists.\nDo you want to replace it ?").arg(m_filename);
            if (QMessageBox::No == QMessageBox::question(this, msg1, msg2, QMessageBox::Yes, QMessageBox::No)) {
                return;
            }
        } else {

            QString msg2 = tr("File %1 already exists and cannot be replaced !").arg(m_filename);
            QMessageBox::warning(this, unable_to_save_msg, msg2);
            return;
        }
    } else if (!QFileInfo(m_path,"").isWritable()) {
        QMessageBox::warning(this, unable_to_save_msg, tr("Cannot write in destination folder ! (permission ?)"));
        return;
    }
    accept();
}

} // namespace matisse
