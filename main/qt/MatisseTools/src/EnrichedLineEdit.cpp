#include "EnrichedLineEdit.h"

using namespace MatisseTools;

EnrichedLineEdit::EnrichedLineEdit(QWidget *parent, QString label, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _lineEdit = new QLineEdit(this);
    _defaultValue = defaultValue;
    setWidget(label, _lineEdit);
    connect(_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(slot_valueChanged()));
}


bool EnrichedLineEdit::currentValueChanged()
{
    return _lineEdit->text().trimmed() != _defaultValue;
}

QString EnrichedLineEdit::currentValue()
{
    return _lineEdit->text().trimmed();
}

//void EnrichedLineEdit::slot_valueChanged()
//{
//    swapColor(currentValueChanged());
//}
