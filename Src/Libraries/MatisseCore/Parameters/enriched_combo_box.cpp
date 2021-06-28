#include "enriched_combo_box.h"

namespace matisse {

EnrichedComboBox::EnrichedComboBox(QWidget *_parent, QString _label, QStringList _values, QString _default_value):
    EnrichedFormWidget(_parent)
{
    m_combo = new QComboBox(this);
    m_combo->setEditable(false);
    m_combo->addItems(_values);
    m_default_value = _default_value;
    m_default_index = _values.indexOf(m_default_value);
    m_combo->setCurrentIndex(m_default_index);
    setWidget(_label, m_combo);

    connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));

}

bool EnrichedComboBox::currentValueChanged()
{
    return (m_combo->currentIndex() != m_initial_index);
}

QString EnrichedComboBox::currentValue()
{
    return m_combo->currentText();
}

qint32 EnrichedComboBox::currentIndex()
{
    return m_combo->currentIndex();
}

void EnrichedComboBox::applyValue(QString _new_value)
{
    int index = m_combo->findText(_new_value, Qt::MatchExactly);

    if (index == -1) {
        qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(_new_value);
        return;
    }

    disconnect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    m_combo->setCurrentIndex(index);
    m_initial_index = index;
    connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
}

void EnrichedComboBox::restoreDefaultValue()
{
    disconnect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    m_combo->setCurrentIndex(m_default_index);
    connect(m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
