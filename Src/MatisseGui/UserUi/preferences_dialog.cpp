#include "preferences_dialog.h"
#include "ui_preferences_dialog.h"

namespace matisse {

PreferencesDialog::PreferencesDialog(QWidget *_parent, MatisseIconFactory *_icon_factory, MatissePreferences *_prefs, bool _allow_programming_mode_activation) :
    QDialog(_parent),
    m_ui(new Ui::PreferencesDialog)
{ 
    m_ui->setupUi(this);

    m_ui->_CB_languageSelect->addItem("FR");
    m_ui->_CB_languageSelect->addItem("EN");

    m_prefs = _prefs;

    m_ui->_LE_importExportPath->setText(m_prefs->importExportPath());
    m_ui->_LE_archivePath->setText(m_prefs->archivePath());
    m_ui->_LE_defaultResultPath->setText(m_prefs->defaultResultPath());
    m_ui->_LE_defaultMosaicPrefix->setText(m_prefs->defaultMosaicFilenamePrefix());
    m_ui->_CK_enableProgrammingMode->setChecked(m_prefs->programmingModeEnabled());

    m_ui->_LE_remoteCommandServerAddress->setText(m_prefs->remoteCommandServer());
    m_ui->_LE_remoteFileServerAddress->setText(m_prefs->remoteFileServer());
    m_ui->_LE_remoteUsername->setText(m_prefs->remoteUsername());
    m_ui->_LE_remoteUserEmail->setText(m_prefs->remoteUserEmail());
    m_ui->_LE_remoteQueueName->setText(m_prefs->remoteQueueName());
    m_ui->_LE_remoteNbOfCpus->setText(QVariant(m_prefs->remoteNbOfCpus()).toString());

    if (!_allow_programming_mode_activation) {
        m_ui->_LA_enableProgrammingMode->setEnabled(false);
        m_ui->_CK_enableProgrammingMode->setEnabled(false);
    }

    if (m_prefs->language() == "FR") {
        m_ui->_CB_languageSelect->setCurrentIndex(0);
    } else { // EN
        m_ui->_CB_languageSelect->setCurrentIndex(1);
    }

    // saving mosaic file prefix buffer
    m_prefix_buffer = m_prefs->defaultMosaicFilenamePrefix();

    // on limite la saisie aux caractères alphanumériques + '-' et '_'
    QRegExpValidator *prefix_val = new QRegExpValidator(QRegExp("[a-zA-Z\\d_\\-]{2,}"), this);
    m_ui->_LE_defaultMosaicPrefix->setValidator(prefix_val);

    /* Set validator for Nb of remote CPUs field */
    QIntValidator *cpus_val = new QIntValidator(this);
    cpus_val->setBottom(1);
    m_ui->_LE_remoteNbOfCpus->setValidator(cpus_val);

    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_importExportPathSelect, SIGNAL(clicked()), this, SLOT(sl_selectDir()));
    connect(m_ui->_PB_archivePathSelect, SIGNAL(clicked()), this, SLOT(sl_selectDir()));
    connect(m_ui->_PB_defaultResultPathSelect, SIGNAL(clicked()), this, SLOT(sl_selectDir()));
    connect(m_ui->_LE_defaultMosaicPrefix, SIGNAL(editingFinished()), this, SLOT(sl_validatePrefixInput()));

    IconizedButtonWrapper *import_export_path_button_wrapper = new IconizedButtonWrapper(m_ui->_PB_importExportPathSelect);
    _icon_factory->attachIcon(import_export_path_button_wrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *archive_path_button_wrapper = new IconizedButtonWrapper(m_ui->_PB_archivePathSelect);
    _icon_factory->attachIcon(archive_path_button_wrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *default_result_path_button_wrapper = new IconizedButtonWrapper(m_ui->_PB_defaultResultPathSelect);
    _icon_factory->attachIcon(default_result_path_button_wrapper, "lnf/icons/Dossier.svg");
}

PreferencesDialog::~PreferencesDialog()
{
    delete m_ui;
}

void PreferencesDialog::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}

void PreferencesDialog::sl_close()
{
    if (sender() == m_ui->_PB_cancel) {
        reject();
    } else {
        QString new_mosaic_prefix = m_ui->_LE_defaultMosaicPrefix->text();
        if (new_mosaic_prefix != m_prefix_buffer) {
            qDebug() << QString("Default mosaic prefix input '%1' is in intermediate validation state, restoring previous : '%2'").arg(new_mosaic_prefix, m_prefix_buffer);
            new_mosaic_prefix = m_prefix_buffer;
        }

        m_prefs->setImportExportPath(m_ui->_LE_importExportPath->text());
        m_prefs->setArchivePath(m_ui->_LE_archivePath->text());
        m_prefs->setDefaultResultPath(m_ui->_LE_defaultResultPath->text());
        m_prefs->setDefaultMosaicFilenamePrefix(new_mosaic_prefix);
        m_prefs->setProgrammingModeEnabled(m_ui->_CK_enableProgrammingMode->isChecked());
        m_prefs->setLanguage(m_ui->_CB_languageSelect->currentText());

        m_prefs->setRemoteCommandServer(m_ui->_LE_remoteCommandServerAddress->text());
        m_prefs->setRemoteFileServer(m_ui->_LE_remoteFileServerAddress->text());
        m_prefs->setRemoteUsername(m_ui->_LE_remoteUsername->text());
        m_prefs->setRemoteUserEmail(m_ui->_LE_remoteUserEmail->text());
        m_prefs->setRemoteQueueName(m_ui->_LE_remoteQueueName->text());
        QVariant cpu_val = m_ui->_LE_remoteNbOfCpus->text();
        int nb_of_cpus = cpu_val.toInt();
        if (nb_of_cpus) {
          m_prefs->setRemoteNbOfCpus(nb_of_cpus);
        }

        accept();
    }
}

void PreferencesDialog::sl_selectDir()
{
    QString sel_dir = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()));

    if (sel_dir.isEmpty()) {
        return;
    }

    QFileInfo dir_info(sel_dir);
    QString dir_path = dir_info.filePath();

    QDir current = QDir::current();
    if (dir_path.startsWith(current.path())) {
        // chemin relatif
        dir_path = current.relativeFilePath(dir_path);
        if (dir_path.isEmpty()) {
            dir_path = ".";
        }

    }

    if (sender() == m_ui->_PB_importExportPathSelect) {
        m_ui->_LE_importExportPath->setText(dir_path);
    } else if (sender() == m_ui->_PB_archivePathSelect) {
        m_ui->_LE_archivePath->setText(dir_path);
    } else if (sender() == m_ui->_PB_defaultResultPathSelect) {
        m_ui->_LE_defaultResultPath->setText(dir_path);
    }
}

void PreferencesDialog::sl_validatePrefixInput()
{
    m_prefix_buffer = m_ui->_LE_defaultMosaicPrefix->text();
}

} // namespace matisse
