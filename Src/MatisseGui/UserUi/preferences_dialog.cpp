#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

using namespace MatisseServer;

PreferencesDialog::PreferencesDialog(QWidget *parent, MatisseIconFactory *iconFactory, MatissePreferences *prefs, bool allowProgrammingModeActivation) :
    QDialog(parent),
    _ui(new Ui::PreferencesDialog)
{ 
    _ui->setupUi(this);

    _ui->_CB_languageSelect->addItem("FR");
    _ui->_CB_languageSelect->addItem("EN");

    _prefs = prefs;

    _ui->_LE_importExportPath->setText(_prefs->importExportPath());
    _ui->_LE_archivePath->setText(_prefs->archivePath());
    _ui->_LE_defaultResultPath->setText(_prefs->defaultResultPath());
    _ui->_LE_defaultMosaicPrefix->setText(_prefs->defaultMosaicFilenamePrefix());
    _ui->_CK_enableProgrammingMode->setChecked(_prefs->programmingModeEnabled());

    _ui->_LE_remoteCommandServerAddress->setText(_prefs->remoteCommandServer());
    _ui->_LE_remoteFileServerAddress->setText(_prefs->remoteFileServer());
    _ui->_LE_remoteUsername->setText(_prefs->remoteUsername());
    _ui->_LE_remoteUserEmail->setText(_prefs->remoteUserEmail());
    _ui->_LE_remoteQueueName->setText(_prefs->remoteQueueName());
    _ui->_LE_remoteNbOfCpus->setText(QVariant(_prefs->remoteNbOfCpus()).toString());

    if (!allowProgrammingModeActivation) {
        _ui->_LA_enableProgrammingMode->setEnabled(false);
        _ui->_CK_enableProgrammingMode->setEnabled(false);
    }

    if (_prefs->language() == "FR") {
        _ui->_CB_languageSelect->setCurrentIndex(0);
    } else { // EN
        _ui->_CB_languageSelect->setCurrentIndex(1);
    }

    // saving mosaic file prefix buffer
    _prefixBuffer = _prefs->defaultMosaicFilenamePrefix();

    // on limite la saisie aux caractères alphanumériques + '-' et '_'
    QRegExpValidator *prefixVal = new QRegExpValidator(QRegExp("[a-zA-Z\\d_\\-]{2,}"), this);
    _ui->_LE_defaultMosaicPrefix->setValidator(prefixVal);

    /* Set validator for Nb of remote CPUs field */
    QIntValidator *cpus_val = new QIntValidator(this);
    cpus_val->setBottom(1);
    _ui->_LE_remoteNbOfCpus->setValidator(cpus_val);

    connect(_ui->_PB_save, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(slot_close()));
    connect(_ui->_PB_importExportPathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_archivePathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_PB_defaultResultPathSelect, SIGNAL(clicked()), this, SLOT(slot_selectDir()));
    connect(_ui->_LE_defaultMosaicPrefix, SIGNAL(editingFinished()), this, SLOT(slot_validatePrefixInput()));
    //connect(_ui->_LE_defaultMosaicPrefix, SIGNAL(textEdited(QString)), this, SLOT(slot_restorePrefixInput(QString)));

    IconizedButtonWrapper *importExportPathButtonWrapper = new IconizedButtonWrapper(_ui->_PB_importExportPathSelect);
    iconFactory->attachIcon(importExportPathButtonWrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *archivePathButtonWrapper = new IconizedButtonWrapper(_ui->_PB_archivePathSelect);
    iconFactory->attachIcon(archivePathButtonWrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *defaultResultPathButtonWrapper = new IconizedButtonWrapper(_ui->_PB_defaultResultPathSelect);
    iconFactory->attachIcon(defaultResultPathButtonWrapper, "lnf/icons/Dossier.svg");
}

PreferencesDialog::~PreferencesDialog()
{
    delete _ui;
}

void PreferencesDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        _ui->retranslateUi(this);
    }
}

void PreferencesDialog::slot_close()
{
    if (sender() == _ui->_PB_cancel) {
        reject();
    } else {
        QString newMosaicPrefix = _ui->_LE_defaultMosaicPrefix->text();
        if (newMosaicPrefix != _prefixBuffer) {
            qDebug() << QString("Default mosaic prefix input '%1' is in intermediate validation state, restoring previous : '%2'").arg(newMosaicPrefix, _prefixBuffer);
            newMosaicPrefix = _prefixBuffer;
        }

        _prefs->setImportExportPath(_ui->_LE_importExportPath->text());
        _prefs->setArchivePath(_ui->_LE_archivePath->text());
        _prefs->setDefaultResultPath(_ui->_LE_defaultResultPath->text());
        _prefs->setDefaultMosaicFilenamePrefix(newMosaicPrefix);
        _prefs->setProgrammingModeEnabled(_ui->_CK_enableProgrammingMode->isChecked());
        _prefs->setLanguage(_ui->_CB_languageSelect->currentText());

        _prefs->setRemoteCommandServer(_ui->_LE_remoteCommandServerAddress->text());
        _prefs->setRemoteFileServer(_ui->_LE_remoteFileServerAddress->text());
        _prefs->setRemoteUsername(_ui->_LE_remoteUsername->text());
        _prefs->setRemoteUserEmail(_ui->_LE_remoteUserEmail->text());
        _prefs->setRemoteQueueName(_ui->_LE_remoteQueueName->text());
        QVariant cpu_val = _ui->_LE_remoteNbOfCpus->text();
        int nb_of_cpus = cpu_val.toInt();
        if (nb_of_cpus) {
          _prefs->setRemoteNbOfCpus(nb_of_cpus);
        }

        accept();
    }
}

void PreferencesDialog::slot_selectDir()
{
    QString selDir = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()));

    if (selDir.isEmpty()) {
        return;
    }

    QFileInfo dirInfo(selDir);
    QString dirPath = dirInfo.filePath();

    QDir current = QDir::current();
    if (dirPath.startsWith(current.path())) {
        // chemin relatif
        dirPath = current.relativeFilePath(dirPath);
        if (dirPath.isEmpty()) {
            dirPath = ".";
        }

    }

    if (sender() == _ui->_PB_importExportPathSelect) {
        _ui->_LE_importExportPath->setText(dirPath);
    } else if (sender() == _ui->_PB_archivePathSelect) {
        _ui->_LE_archivePath->setText(dirPath);
    } else if (sender() == _ui->_PB_defaultResultPathSelect) {
        _ui->_LE_defaultResultPath->setText(dirPath);
    }
}

void PreferencesDialog::slot_validatePrefixInput()
{
    _prefixBuffer = _ui->_LE_defaultMosaicPrefix->text();
}


// Ne marche pas car appelé à chaque saisie de caractère
// Il faudrait traquer la sortie de focus en surchargeant QLineEdit

//void PreferencesDialog::slot_restorePrefixInput(QString newText)
//{
//    int cursor = newText.size();

//    QValidator::State valState = _ui->_LE_defaultMosaicPrefix->validator()->validate(newText, cursor);

//    if (valState != QValidator::Acceptable) {
//        qDebug() << QString("Default mosaic prefix input '%1' is in intermediate state, restoring previous : '%2'").arg(newText, _prefixBuffer);
//        _ui->_LE_defaultMosaicPrefix->setText(_prefixBuffer);
//    }
//}
