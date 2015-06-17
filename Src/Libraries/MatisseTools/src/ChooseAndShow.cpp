#include "ChooseAndShow.h"
#include "ui_ChooseAndShow.h"

using namespace MatisseTools;
ChooseAndShow::ChooseAndShow(QWidget *parent, QString label, QString buttonText, QString defaultValue) :
    EnrichedFormWidget(parent),
    _ui(new Ui::ChooseAndShow)
{
    _ui->setupUi(this);

    _defaultValue = defaultValue;

    setWidget(label, this);
    _ui->_PB_choose->setText(buttonText);
    connect(_ui->_PB_choose, SIGNAL(clicked()), this, SLOT(slot_clicked()));
    connect(_ui->_LE_show, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

ChooseAndShow::~ChooseAndShow()
{
    delete _ui;
}

void ChooseAndShow::slot_clicked() {
    QString choiceType = _ui->_PB_choose->text();
    if (choiceType.toLower().startsWith("fi")) {
        // Ca marche pour file et fichier...
        _ui->_LE_show->setText(QFileDialog::getOpenFileName(this, "Choix d'un fichier..."));
    } else {
        _ui->_LE_show->setText(QFileDialog::getExistingDirectory(this, "Choix d'un répertoire..."));
    }

    slot_valueChanged();
}

bool ChooseAndShow::currentValueChanged()
{
    return (currentValue() != _defaultValue);
}

QString ChooseAndShow::currentValue()
{
    return  _ui->_LE_show->text();
}

QString ChooseAndShow::buttonText() {
    return _ui->_PB_choose->text();
}
