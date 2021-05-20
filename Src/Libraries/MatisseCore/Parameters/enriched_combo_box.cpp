#include "enriched_combo_box.h"

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
    return (_combo->currentIndex() != _initialIndex);
}

QString EnrichedComboBox::currentValue()
{
    return _combo->currentText();
}

qint32 EnrichedComboBox::currentIndex()
{
    return _combo->currentIndex();
}

void EnrichedComboBox::applyValue(QString newValue)
{
    int index = _combo->findText(newValue, Qt::MatchExactly);

    if (index == -1) {
        qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(newValue);
        return;
    }

    disconnect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    _combo->setCurrentIndex(index);
    _initialIndex = index;
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}

void EnrichedComboBox::restoreDefaultValue()
{
    disconnect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    _combo->setCurrentIndex(_defaultIndex);
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}
