#include "EnrichedFileChooser.h"

using namespace MatisseTools;

EnrichedFileChooser::EnrichedFileChooser(QWidget *parent, QString label, ParameterShow type, QString defaultValue) :
    EnrichedFormWidget(parent)
{
    QWidget * comboWidget = new QWidget(this);
    QHBoxLayout * layout = new QHBoxLayout(comboWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    _lineEdit = new QLineEdit(defaultValue);
    QPushButton * button = new QPushButton();
    button->setObjectName("_PB_paramFileSelect");

    _type = type;

    if (_type == FILE_SELECTOR_RELATIVE || _type == FILE_SELECTOR_ABSOLUTE) {
        button->setIcon(QIcon(":/qss_icons/icons/File.svg"));
    } else {
        button->setIcon(QIcon(":/qss_icons/icons/Dossier.svg"));
    }

    layout->addWidget(_lineEdit);
    layout->addWidget(button);

    _defaultValue = defaultValue;

    setWidget(label, comboWidget);
    connect(button, SIGNAL(clicked()), this, SLOT(slot_clicked()));
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

bool EnrichedFileChooser::currentValueChanged()
{
    return _lineEdit->text().trimmed() != _defaultValue;
}

QString EnrichedFileChooser::currentValue()
{
    return _lineEdit->text().trimmed();
}

void EnrichedFileChooser::setValue(QString newValue)
{
    _lineEdit->setText(newValue);
}

void EnrichedFileChooser::restoreDefaultValue()
{
    _lineEdit->setText(_defaultValue);
}

void EnrichedFileChooser::slot_clicked() {
    QString retFile;
    if ((_type == FILE_SELECTOR_RELATIVE) || (_type == FILE_SELECTOR_ABSOLUTE)) {
        retFile = QFileDialog::getOpenFileName(qobject_cast<QWidget *>(sender()), "Choix d'un fichier...");
    } else {
        retFile = QFileDialog::getExistingDirectory(qobject_cast<QWidget *>(sender()), "Choix d'un répertoire...");
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
