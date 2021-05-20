#include "enriched_file_chooser.h"

using namespace MatisseTools;

EnrichedFileChooser::EnrichedFileChooser(QWidget *parent,  MatisseIconFactory *iconFactory, QString label, ParameterShow type, QString defaultValue) :
    EnrichedFormWidget(parent)
{
    QWidget * comboWidget = new QWidget(this);
    QHBoxLayout * layout = new QHBoxLayout(comboWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(3);
    _lineEdit = new QLineEdit(defaultValue);
    QPushButton * button = new QPushButton();
    button->setObjectName("_PB_paramFileSelect");

    _type = type;

    IconizedButtonWrapper *buttonWrapper = new IconizedButtonWrapper(button);

    if (_type == FILE_SELECTOR_RELATIVE || _type == FILE_SELECTOR_ABSOLUTE) {
        iconFactory->attachIcon(buttonWrapper, "lnf/icons/File.svg", false, false);
//        button->setIcon(QIcon(":/qss_icons/icons/File.svg"));
    } else {
        iconFactory->attachIcon(buttonWrapper, "lnf/icons/Dossier.svg", false, false);
//        button->setIcon(QIcon(":/qss_icons/icons/Dossier.svg"));
    }

    layout->addWidget(_lineEdit);
    layout->addWidget(button);

    _defaultValue = defaultValue;

    setWidget(label, comboWidget);
    connect(button, SIGNAL(clicked()), this, SLOT(slot_clicked()));
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

QString EnrichedFileChooser::currentValue()
{
    return _lineEdit->text().trimmed();
}

void EnrichedFileChooser::applyValue(QString newValue)
{
    disconnect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
    _lineEdit->setText(newValue);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

void EnrichedFileChooser::restoreDefaultValue()
{
    disconnect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
    _lineEdit->setText(_defaultValue);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

void EnrichedFileChooser::slot_clicked() {
    QString retFile;
    if ((_type == FILE_SELECTOR_RELATIVE) || (_type == FILE_SELECTOR_ABSOLUTE)) {
        retFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), tr("Choose file..."));
    } else {
        retFile = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()), tr("Choose directory..."));
    }
    if (!retFile.isEmpty()) {
        if ((_type == FILE_SELECTOR_RELATIVE) || (_type == DIR_SELECTOR_RELATIVE)) {
            QFileInfo fileInfo(retFile);
            retFile = fileInfo.fileName();
        }
        _lineEdit->setText(retFile);
        slot_valueChanged();
    }
}
