#include "EnrichedComboBox.h"

using namespace MatisseTools;

EnrichedComboBox::EnrichedComboBox(QWidget *parent, QString label, QStringList values, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _combo = new QComboBox(this);
    _combo->setEditable(false);
    _combo->addItems(values);
    _defaultValue = defaultValue;
    _defaultIndex = values.indexOf(_defaultValue);
    _combo->setCurrentIndex(_defaultIndex);
    setWidget(label, _combo);

    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));

}

bool EnrichedComboBox::currentValueChanged()
{
    return (_combo->currentIndex() != _defaultIndex);
}

QString EnrichedComboBox::currentValue()
{
    return _combo->currentText();
}

qint32 EnrichedComboBox::currentIndex()
{
    return _combo->currentIndex();
}
