#include "job_dialog.h"
#include "ui_job_dialog.h"

using namespace MatisseServer;

JobDialog::JobDialog(QWidget *parent, MatisseIconFactory *iconFactory, KeyValueList *keyValues, QString jobsPath, QStringList existingJobNames, QStringList archivedJobNames) :
    QDialog(parent),
    _ui(new Ui::JobDialog),
    _keyValues(keyValues),
    _isRealTime(false),
    _jobsPath(jobsPath),
    _existingJobNames(existingJobNames),
    _archivedJobNames(archivedJobNames)
{
    _ui->setupUi(this);

    _ui->_LE_name->setValidator(new QRegExpValidator(QRegExp("\\w+(\\s|\\w)+\\w")));

    if (!keyValues->getKeys().contains(DATASET_PARAM_DATASET_DIR)) {
        // RT processing chain : fields for dataset dir and nav file are disabled
        _isRealTime = true;
        _ui->_LA_dataPath->setEnabled(false);
        _ui->_LE_dataPath->setEnabled(false);
        _ui->_PB_dataPath->setEnabled(false);
        _ui->_LA_navigationFile->setEnabled(false);
        _ui->_LE_navigationFile->setEnabled(false);
        _ui->_PB_navigationFile->setEnabled(false);
    } else { // Post-processing
        _ui->_LE_dataPath->setText(keyValues->getValue(DATASET_PARAM_DATASET_DIR));
        _ui->_LE_navigationFile->setText(keyValues->getValue(DATASET_PARAM_NAVIGATION_FILE));
    }

    _ui->_LE_resultPath->setText(keyValues->getValue(DATASET_PARAM_OUTPUT_DIR));
    _ui->_LE_outputFile->setText(keyValues->getValue(DATASET_PARAM_OUTPUT_FILENAME));

    connect(_ui->_LE_name, SIGNAL(textEdited(QString)), this, SLOT(slot_formatName(QString)));
    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_dataPath, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    //connect(_ui->_PB_resultPath, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_navigationFile, SIGNAL(clicked()), this, SLOT(slot_selectFile()));

    IconizedButtonWrapper *dataPathButtonWrapper = new IconizedButtonWrapper(_ui->_PB_dataPath);
    iconFactory->attachIcon(dataPathButtonWrapper, "lnf/icons/Dossier.svg");

    //IconizedButtonWrapper *resultPathButtonWrapper = new IconizedButtonWrapper(_ui->_PB_resultPath);
    //iconFactory->attachIcon(resultPathButtonWrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *navFileButtonWrapper = new IconizedButtonWrapper(_ui->_PB_navigationFile);
    iconFactory->attachIcon(navFileButtonWrapper, "lnf/icons/File.svg");
}

JobDialog::~JobDialog()
{
    delete _ui;
}

void JobDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
}


//QString JobDialog::newJobName(QWidget *parent, KeyValueList *keyValues, QString jobsPath)
//{
//    QString jobName;

//    JobDialog * newJobDialog = new JobDialog(parent, keyValues, jobsPath);
//    if (newJobDialog->exec() == Accepted) {
//        jobName = keyValues->getValue("name");
//    }

//    return jobName;
//}

void JobDialog::slot_formatName(QString text) {
    text.replace(" ", "_");
    _ui->_LE_name->setText(text);
}

void JobDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        // normalisation du nom de job
        QString name = _ui->_LE_name->text().trimmed().toLower();
        //name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_—+=|:;<>«»,.?/{}\'\"\\\[\\\]\\\\]")));
        name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Cannot save..."), tr("You must give a name."));
            return;
        }


        if (_existingJobNames.contains(name)) {
            QMessageBox::warning(this, tr("Cannot save..."),
                                 tr("Name '%1' is already used.").arg(name));
            return;
        }

        if (_archivedJobNames.contains(name)) {
            QMessageBox::warning(this, tr("Cannot save..."),
                                 tr("Name '%1' is used b an archived task.").arg(name));
            return;
        }

        QString filename(name);
        filename.replace(" ", "_").append(".xml");
        QFileInfo info(_jobsPath + QDir::separator() + filename);
        if (info.exists()) {
            /* Technically inconsistent state */
            qCritical() << QString("The file %1 was found, could not create job %2").arg(info.absoluteFilePath()).arg(name);
            // Le nom est déjà utilisé
            QMessageBox::critical(this, tr("Cannot save..."), tr("File '%1' already exists.").arg(info.absoluteFilePath()));
            return;
        }

        _keyValues->set("filename", info.absoluteFilePath());
        _keyValues->set("name", name);
        _keyValues->set("comment", _ui->_TXT_comments->toPlainText().trimmed());

        if (!_isRealTime) {
            _keyValues->set(DATASET_PARAM_DATASET_DIR, _ui->_LE_dataPath->text());
            _keyValues->set(DATASET_PARAM_NAVIGATION_FILE, _ui->_LE_navigationFile->text());
        }
        _keyValues->set(DATASET_PARAM_OUTPUT_DIR, _ui->_LE_resultPath->text());
        _keyValues->set(DATASET_PARAM_OUTPUT_FILENAME, _ui->_LE_outputFile->text());

        accept();
    }
}

void JobDialog::slot_selectDir()
{
    QDir dataRoot("./");
    QString fieldText;
    QString caption;
    QDir current = dataRoot;

    bool isDataPath = (sender() == _ui->_PB_dataPath);

    if (isDataPath) {
        caption = tr("Select data path");
        fieldText = _ui->_LE_dataPath->text();

    } else {
        caption = tr("Output folder");
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
            dirPath = "./";
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
    QDir dataRoot("./");
    QString currentPath = dataRoot.path();
    QString fieldText;

    fieldText = _ui->_LE_navigationFile->text();

    // Dossier parent du fichier courant
    if (!fieldText.isEmpty()) {
        QFileInfo currentFile(fieldText);
        if (currentFile.exists()) {
            currentPath = currentFile.absoluteFilePath();
        }
    }

    selFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Select navigation file"), currentPath, "Nav files (*.dim2 *.txt)");

    if (selFile.isEmpty()) {
        return;
    }

    // Chemin relatif si fichier contenu dans l'arborescence de données par défaut
    QString rootPath = dataRoot.absolutePath();
    if (selFile.startsWith(rootPath)) {
        selFile = dataRoot.relativeFilePath(selFile);
    }

    _ui->_LE_navigationFile->setText(selFile);
}
