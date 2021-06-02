#include "enriched_line_edit.h"

namespace matisse {

EnrichedLineEdit::EnrichedLineEdit(QWidget *parent, QString label, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _lineEdit = new QLineEdit(defaultValue, this);
    _defaultValue = defaultValue;
    setWidget(label, _lineEdit);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}


//bool EnrichedLineEdit::currentValueChanged()
//{
//    return _lineEdit->text().trimmed() != _defaultValue;
//}

QString EnrichedLineEdit::currentValue()
{
    return _lineEdit->text().trimmed();
}

void EnrichedLineEdit::restoreDefaultValue()
{
    disconnect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
    _lineEdit->setText(_defaultValue);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

void EnrichedLineEdit::applyValue(QString newValue)
{
    disconnect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
    _lineEdit->setText(newValue);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}

//void EnrichedLineEdit::slot_valueChanged()
//{
//    swapColor(currentValueChanged());
//}

} // namespace matisse
