#include "EnrichedListBox.h"

using namespace MatisseTools;

EnrichedListBox::EnrichedListBox(QWidget *parent, QString label, QStringList values, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _defaultValue = defaultValue;
    _list = new QListWidget(this);
    _list->addItems(values);
    _list->setCurrentRow(values.indexOf(_defaultValue));
    setWidget(label, _list);
    connect(_list, SIGNAL(currentRowChanged(int)), this, SLOT(slot_valueChanged()));
}

bool EnrichedListBox::currentValueChanged()
{
    return currentValue() != _defaultValue;
}

QString EnrichedListBox::currentValue()
{
    return _list->currentItem()->text();
}
