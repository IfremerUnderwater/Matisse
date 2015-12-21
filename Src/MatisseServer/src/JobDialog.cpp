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

    if (!keyValues->getKeys().contains("dataPath")) {
        // Traitement TR : on désactive le chemin des données et fichier de nav
        _ui->_LA_dataPath->setEnabled(false);
        _ui->_LE_dataPath->setEnabled(false);
        _ui->_PB_dataPath->setEnabled(false);
        _ui->_LA_navigationFile->setEnabled(false);
        _ui->_LE_navigationFile->setEnabled(false);
        _ui->_PB_navigationFile->setEnabled(false);
    }
    _ui->_LE_resultPath->setText(keyValues->getValue("resultPath"));
    _ui->_LE_outputFile->setText(keyValues->getValue("outputFile"));

    _ui->_LE_dataPath->setReadOnly(true);
    _ui->_LE_resultPath->setReadOnly(true);
    _ui->_LE_outputFile->setReadOnly(true);
    _ui->_LE_navigationFile->setReadOnly(true);

    connect(_ui->_LE_name, SIGNAL(textEdited(QString)), this, SLOT(slot_formatName(QString)));
    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_dataPath, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_resultPath, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_outputFile, SIGNAL(clicked()), this, SLOT(slot_selectFile()));
    connect(_ui->_PB_navigationFile, SIGNAL(clicked()), this, SLOT(slot_selectFile()));
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

void JobDialog::slot_selectDir()
{
    QDir dataRoot(".");
    QString fieldText;
    QString caption;
    QDir current = dataRoot;

    bool isDataPath = (sender() == _ui->_PB_dataPath);

    if (isDataPath) {
        caption = trUtf8("Sélectionner chemin des données");
        fieldText = _ui->_LE_dataPath->text();

    } else {
        caption = trUtf8("Sélectionner chemin du résultat");
        fieldText = _ui->_LE_resultPath->text();
    }

    // Dossier courant
    if (!fieldText.isEmpty()) {
        QFileInfo currentFile(fieldText);
        if (currentFile.exists() && currentFile.isDir()) {
            current = QDir(currentFile.absoluteFilePath());
        }
    }

    QString selDir = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()), caption, current.path());

    if (selDir.isEmpty()) {
        return;
    }

    QFileInfo dirInfo(selDir);
    QString dirPath = dirInfo.filePath();

    QString rootPath = dataRoot.absolutePath();

    if (dirPath.startsWith(rootPath)) {
        // chemin relatif
        dirPath = dataRoot.relativeFilePath(dirPath);
        if (dirPath.isEmpty()) {
            dirPath = ".";
        }
    }

    if (isDataPath) {
        _ui->_LE_dataPath->setText(dirPath);
    } else {
        _ui->_LE_resultPath->setText(dirPath);
    }
}

void JobDialog::slot_selectFile()
{
    QString selFile;
    QDir dataRoot(".");
    QString currentPath = dataRoot.path();
    QString fieldText;
    bool isNavFile = (sender() == _ui->_PB_navigationFile);

    if (isNavFile) {
        fieldText = _ui->_LE_navigationFile->text();
    } else {
        fieldText = _ui->_LE_outputFile->text();
    }

    // Dossier parent du fichier courant
    if (!fieldText.isEmpty()) {
        QFileInfo currentFile(fieldText);
        if (currentFile.exists()) {
            currentPath = currentFile.absoluteFilePath();
        }
    }

    if (isNavFile) {
        selFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), trUtf8("Sélectionner fichier de navigation"), currentPath);
    } else {
        selFile = QFileDialog::getSaveFileName(qobject_cast<QWidget *>(sender()), trUtf8("Créer fichier de sortie"), currentPath, trUtf8("Fichier sortie mosaïque (*.xml)"));
    }

    if (selFile.isEmpty()) {
        return;
    }

    // Chemin relatif si fichier contenu dans l'arborescence de données par défaut
    QString rootPath = dataRoot.absolutePath();
    if (selFile.startsWith(rootPath)) {
        selFile = dataRoot.relativeFilePath(selFile);
    }

    if (isNavFile) {
        _ui->_LE_navigationFile->setText(selFile);
    } else {
        _ui->_LE_outputFile->setText(selFile);
    }

}
