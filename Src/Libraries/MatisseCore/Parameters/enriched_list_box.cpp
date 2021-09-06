#include "enriched_list_box.h"

namespace matisse {

EnrichedListBox::EnrichedListBox(QWidget *_parent, QString _label, QStringList _values, QString _default_value):
    EnrichedFormWidget(_parent)
{
    m_default_value = _default_value;
    m_list = new QListWidget(this);
    m_list->addItems(_values);
    m_default_value_index = _values.indexOf(m_default_value);
    m_list->setCurrentRow(m_default_value_index);
    setWidget(_label, m_list);
    connect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(sl_valueChanged()));
}


QString EnrichedListBox::currentValue()
{
    return m_list->currentItem()->text();
}

void EnrichedListBox::applyValue(QString _new_value)
{
    QList<QListWidgetItem *> items = m_list->findItems(_new_value, Qt::MatchExactly);

    if (items.isEmpty()) {
        qWarning() << QString("Value '%1' not found among list box items, skipping assignment...").arg(_new_value);
        return;
    }

    disconnect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(sl_valueChanged()));
    m_list->setCurrentItem(items.at(0));
    connect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(sl_valueChanged()));
}

void EnrichedListBox::restoreDefaultValue()
{
    disconnect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(sl_valueChanged()));
    m_list->setCurrentRow(m_default_value_index);
    connect(m_list, SIGNAL(currentRowChanged(int)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
