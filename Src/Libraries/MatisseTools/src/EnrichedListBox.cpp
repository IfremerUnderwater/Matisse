#include "EnrichedListBox.h"

using namespace MatisseTools;

EnrichedListBox::EnrichedListBox(QWidget *parent, QString label, QStringList values, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _defaultValue = defaultValue;
    _list = new QListWidget(this);
    _list->addItems(values);
    _defaultValueIndex = values.indexOf(_defaultValue);
    _list->setCurrentRow(_defaultValueIndex);
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

void EnrichedListBox::setValue(QString newValue)
{
    QList<QListWidgetItem *> items = _list->findItems(newValue, Qt::MatchExactly);

    if (items.isEmpty()) {
        qWarning() << QString("Value '%1' not found among list box items, skipping assignment...").arg(newValue);
        return;
    }

    _list->setCurrentItem(items.at(0));
}

void EnrichedListBox::restoreDefaultValue()
{
    _list->setCurrentRow(_defaultValueIndex);
}