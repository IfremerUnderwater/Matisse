#include "job_dialog.h"
#include "ui_job_dialog.h"

#include "nav_commons.h"

namespace matisse {

JobDialog::JobDialog(QWidget *_parent, MatisseIconFactory *_icon_factory, KeyValueList *_key_values, QString _jobs_path, QStringList _existing_job_names, QStringList _archived_job_names) :
    QDialog(_parent),
    m_ui(new Ui::JobDialog),
    m_key_values(_key_values),
    m_jobs_path(_jobs_path),
    m_existing_job_names(_existing_job_names),
    m_archived_job_names(_archived_job_names)
{
    m_ui->setupUi(this);

    m_ui->_LE_name->setValidator(new QRegExpValidator(QRegExp("\\w+(\\s|\\w)+\\w")));

    if (!_key_values->getKeys().contains(DATASET_PARAM_DATASET_DIR)) {
        qCritical() << "Dataset dir param not found : mandatory for post-processing";
    } else { // Post-processing
        m_ui->_LE_dataPath->setText(_key_values->getValue(DATASET_PARAM_DATASET_DIR));
        m_ui->_LE_navigationFile->setText(_key_values->getValue(DATASET_PARAM_NAVIGATION_FILE));
    }

    m_ui->_LE_resultPath->setText(_key_values->getValue(DATASET_PARAM_OUTPUT_DIR));
    m_ui->_LE_outputFile->setText(_key_values->getValue(DATASET_PARAM_OUTPUT_FILENAME));

    connect(m_ui->_LE_name, SIGNAL(textEdited(QString)), this, SLOT(sl_formatName(QString)));
    connect(m_ui->_PB_save, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_cancel, SIGNAL(clicked()), this, SLOT(sl_close()));
    connect(m_ui->_PB_dataPath, SIGNAL(clicked()), this, SLOT(sl_selectDir()));
    connect(m_ui->_PB_navigationFile, SIGNAL(clicked()), this, SLOT(sl_selectFile()));

    IconizedButtonWrapper *data_path_button_wrapper = new IconizedButtonWrapper(m_ui->_PB_dataPath);
    _icon_factory->attachIcon(data_path_button_wrapper, "lnf/icons/Dossier.svg");

    IconizedButtonWrapper *nav_file_button_wrapper = new IconizedButtonWrapper(m_ui->_PB_navigationFile);
    _icon_factory->attachIcon(nav_file_button_wrapper, "lnf/icons/File.svg");
}

JobDialog::~JobDialog()
{
    delete m_ui;
}

void JobDialog::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        m_ui->retranslateUi(this);
    }
}


void JobDialog::sl_formatName(QString _text) {
    _text.replace(" ", "_");
    m_ui->_LE_name->setText(_text);
}

void JobDialog::sl_close()
{
    if (sender() == m_ui->_PB_cancel) {
        reject();
    } else {
        // normalisation du nom de job
        QString name = m_ui->_LE_name->text().trimmed().toLower();
        name.remove(QRegExp(QString::fromUtf8("[-`~!@#$%^&*()_+|~=`{}\\[\\]:\";'<>?,.\\\\/]")));

        if (name.isEmpty()) {
            QMessageBox::warning(this, tr("Cannot save..."), tr("You must give a name."));
            return;
        }


        if (m_existing_job_names.contains(name)) {
            QMessageBox::warning(this, tr("Cannot save..."),
                                 tr("Name '%1' is already used.").arg(name));
            return;
        }

        if (m_archived_job_names.contains(name)) {
            QMessageBox::warning(this, tr("Cannot save..."),
                                 tr("Name '%1' is used b an archived task.").arg(name));
            return;
        }

        if (!m_ui->_LE_navigationFile->text().endsWith("dim2"))
        {
            QMessageBox::warning(this, tr("Wrong navigation file"), tr("The navigation file you selected is not a dim2 file. If you want your model to be scaled you should use a dim2 file or have exif metadata in the images."));
        }

        QString filename(name);
        filename.replace(" ", "_").append(".xml");
        QFileInfo info(m_jobs_path + QDir::separator() + filename);
        if (info.exists()) {
            /* Technically inconsistent state */
            qCritical() << QString("The file %1 was found, could not create job %2").arg(info.absoluteFilePath()).arg(name);
            // Le nom est déjà utilisé
            QMessageBox::critical(this, tr("Cannot save..."), tr("File '%1' already exists.").arg(info.absoluteFilePath()));
            return;
        }

        m_key_values->set("filename", info.absoluteFilePath());
        m_key_values->set("name", name);
        m_key_values->set("comment", m_ui->_TXT_comments->toPlainText().trimmed());

        m_key_values->set(DATASET_PARAM_DATASET_DIR, m_ui->_LE_dataPath->text());
        m_key_values->set(DATASET_PARAM_NAVIGATION_FILE, m_ui->_LE_navigationFile->text());
        m_key_values->set(DATASET_PARAM_OUTPUT_DIR, m_ui->_LE_resultPath->text());
        m_key_values->set(DATASET_PARAM_OUTPUT_FILENAME, m_ui->_LE_outputFile->text());

        accept();
    }
}

void JobDialog::sl_selectDir()
{
    QDir data_root("./");
    QString field_text;
    QString caption;
    QDir current = data_root;

    bool is_data_path = (sender() == m_ui->_PB_dataPath);

    if (is_data_path) {
        caption = tr("Select data path");
        field_text = m_ui->_LE_dataPath->text();

    } else {
        caption = tr("Output folder");
        field_text = m_ui->_LE_resultPath->text();
    }

    // Dossier courant
    if (!field_text.isEmpty()) {
        QFileInfo current_file(field_text);
        if (current_file.exists() && current_file.isDir()) {
            current = QDir(current_file.absoluteFilePath());
        }
    }

    QString sel_dir = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()), caption, current.path());

    if (sel_dir.isEmpty()) {
        return;
    }

    QFileInfo dir_info(sel_dir);
    QString dir_path = dir_info.filePath();

    QString root_path = data_root.absolutePath();

    if (dir_path.startsWith(root_path)) {
        // chemin relatif
        dir_path = data_root.relativeFilePath(dir_path);
        if (dir_path.isEmpty()) {
            dir_path = "./";
        }
    }

    if (is_data_path) {
        m_ui->_LE_dataPath->setText(dir_path);
    } else {
        m_ui->_LE_resultPath->setText(dir_path);
    }
}

void JobDialog::sl_selectFile()
{
    QString sel_file;
    QDir data_root(m_ui->_LE_dataPath->text());
    QString current_path = data_root.path();
    QString field_text;

    field_text = m_ui->_LE_navigationFile->text();

    // Dossier parent du fichier courant
    if (!field_text.isEmpty()) {
        QFileInfo current_file(field_text);
        if (current_file.exists()) {
            current_path = current_file.absoluteFilePath();
        }
    }

    // sel_file = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Select navigation file"), current_path, "Nav files (*.dim2 *.txt)");
    sel_file = QFileDialog::getOpenFileName(
                qobject_cast<QWidget *>(sender()),
                tr("Select navigation file"),
                current_path,
                tr("Nav files (%1)").arg(NAV_FILE_TYPE_FILTER));

    if (sel_file.isEmpty()) {
        return;
    }

    // Chemin relatif si fichier contenu dans l'arborescence de données par défaut
    QString root_path = data_root.absolutePath();
    if (sel_file.startsWith(root_path)) {
        sel_file = data_root.relativeFilePath(sel_file);
    }

    m_ui->_LE_navigationFile->setText(sel_file);
}

} // namespace matisse
