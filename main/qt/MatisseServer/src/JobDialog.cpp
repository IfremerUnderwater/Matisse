#include "JobDialog.h"
#include "ui_JobDialog.h"

using namespace MatisseServer;

JobDialog::JobDialog(QWidget *parent, KeyValueList *keyValues, QString jobsPath) :
    QDialog(parent),
    _ui(new Ui::JobDialog),
    _keyValues(keyValues),
    _jobsPath(jobsPath)
{
    _ui->setupUi(this);

    _ui->_LE_name->setValidator(new QRegExpValidator(QRegExp("\\w+(\\s|\\w)+\\w")));
    connect(_ui->_LE_name, SIGNAL(textEdited(QString)), this, SLOT(slot_formatName(QString)));
    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
}

JobDialog::~JobDialog()
{
    delete _ui;
}

QString JobDialog::newJobName(QWidget *parent, KeyValueList *keyValues, QString jobsPath)
{
    QString jobName;

    JobDialog * newJobDialog = new JobDialog(parent, keyValues, jobsPath);
    if (newJobDialog->exec() == Accepted) {
        jobName = keyValues->getValue("name");
    }

    return jobName;
}

void JobDialog::slot_formatName(QString text) {
    text.replace(" ", "_");
    _ui->_LE_name->setText(text);
}

void JobDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        QString name = _ui->_LE_name->text().trimmed().toLower();
        QString filename(name);
        filename.replace(" ", "_").append(".xml");
        QFileInfo info(_jobsPath + QDir::separator() + filename);
        if (info.exists()) {
            // Le nom est déjà utilisé
            QMessageBox::warning(this, "Enregistrement impossible...", "Un travail sous ce nom existe déjà...");
            return;
        }
        _keyValues->set("name", name);
        _keyValues->set("comment", _ui->_TXT_comments->toPlainText().trimmed());
        _keyValues->set("filename", filename);

        accept();
    }
}
