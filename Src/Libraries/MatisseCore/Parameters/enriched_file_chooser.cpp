#include "enriched_file_chooser.h"

namespace matisse {

EnrichedFileChooser::EnrichedFileChooser(QWidget *_parent,  MatisseIconFactory *_icon_factory, QString _label, eParameterShow _type, QString _default_value) :
    EnrichedFormWidget(_parent)
{
    QWidget * combo_widget = new QWidget(this);
    QHBoxLayout * layout = new QHBoxLayout(combo_widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    m_line_edit = new QLineEdit(_default_value);
    QPushButton * button = new QPushButton();
    button->setObjectName("_PB_paramFileSelect");

    m_type = _type;

    IconizedButtonWrapper *button_wrapper = new IconizedButtonWrapper(button);

    if (m_type == FILE_SELECTOR_RELATIVE || m_type == FILE_SELECTOR_ABSOLUTE) {
        _icon_factory->attachIcon(button_wrapper, "lnf/icons/File.svg", false, false);
    } else {
        _icon_factory->attachIcon(button_wrapper, "lnf/icons/Dossier.svg", false, false);
    }

    layout->addWidget(m_line_edit);
    layout->addWidget(button);

    m_default_value = _default_value;

    setWidget(_label, combo_widget);
    connect(button, SIGNAL(clicked()), this, SLOT(sl_clicked()));
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}

QString EnrichedFileChooser::currentValue()
{
    return m_line_edit->text().trimmed();
}

void EnrichedFileChooser::applyValue(QString _new_value)
{
    disconnect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
    m_line_edit->setText(_new_value);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}

void EnrichedFileChooser::restoreDefaultValue()
{
    disconnect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
    m_line_edit->setText(m_default_value);
    connect(m_line_edit, SIGNAL(textEdited(QString)), this, SLOT(sl_valueChanged()));
}

void EnrichedFileChooser::sl_clicked() {
    QString ret_file;
    if ((m_type == FILE_SELECTOR_RELATIVE) || (m_type == FILE_SELECTOR_ABSOLUTE)) {
        ret_file = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Choose file..."));
    } else {
        ret_file = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()), tr("Choose directory..."));
    }
    if (!ret_file.isEmpty()) {
        if ((m_type == FILE_SELECTOR_RELATIVE) || (m_type == DIR_SELECTOR_RELATIVE)) {
            QFileInfo fileInfo(ret_file);
            ret_file = fileInfo.fileName();
        }
        m_line_edit->setText(ret_file);
        sl_valueChanged();
    }
}

} // namespace matisse
