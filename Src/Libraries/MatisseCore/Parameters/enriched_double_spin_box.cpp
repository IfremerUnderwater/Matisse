#include "enriched_double_spin_box.h"
#include "graphical_charter.h"

namespace matisse {

EnrichedDoubleSpinBox::EnrichedDoubleSpinBox(QWidget *_parent, QString _label, QString _min_value, QString _max_value, QString _default_value):
    EnrichedDecimalValueWidget(_parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();
    m_spin = new QDoubleSpinBox(this);
    m_spin->setFixedWidth(graph_chart.dpiScaled(PARAM_SPINBOX_WIDTH));
    _min_value = _min_value.trimmed().toLower();
    _max_value = _max_value.trimmed().toLower();
    QString special_value;

    quint8 default_precision = PRECISION_DEFAULT;
    double increment = qPow(10, -1 * default_precision);

    bool ok;

    if (_min_value.startsWith("-inf")) {
        special_value = "-inf";
        m_min_value_real = MIN_REAL;
    } else {
        m_min_value_real = _min_value.toDouble();
    }

    if (_max_value.startsWith("inf")) {
        special_value = "inf";
        m_max_value_real = MAX_REAL;
        if (qAbs(m_min_value_real - MIN_REAL) > increment) {
            m_min_value_real -= increment;
        }
    } else {
        m_max_value_real = _max_value.toInt();
    }


    m_spin->setRange(m_min_value_real, m_max_value_real);
    m_spin->setWrapping(true);
    m_spin->setSingleStep(increment);
    m_spin->setDecimals(default_precision);

    if (!special_value.isEmpty()) {
        m_spin->setSpecialValueText(special_value);
    }
    m_default_value = _default_value;
    double default_value_real = _default_value.toDouble(&ok);
    if (!ok) {
        qWarning() << QString("Could not convert default value '%1' to real, using min value as default").arg(m_default_value);

        m_spin->setValue(m_min_value_real);
        m_default_value = QString::number(m_min_value_real);
    } else {
        m_spin->setValue(default_value_real);
    }

    setWidget(_label, m_spin);

    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));

}

//bool EnrichedDoubleSpinBox::currentValueChanged()
//{
//    return _spin->text() != _defaultValue;
//}

QString EnrichedDoubleSpinBox::currentValue()
{
    return m_spin->text();
}

void EnrichedDoubleSpinBox::applyValue(QString _new_value)
{
    bool ok;

    double value_real = _new_value.toDouble(&ok);
    if (!ok) {
        qWarning() << QString("Error converting '%1' to double for double spin box value assignment, skipping...").arg(_new_value);
        return;
    }

    if (value_real > m_max_value_real) {
        qWarning() << QString("Value '%1' greater than max value '%2', skipping...").arg(value_real).arg(m_max_value_real);
        return;
    }

    if (value_real < m_min_value_real) {
        qWarning() << QString("Value '%1' lower than min value '%2', skipping...").arg(value_real).arg(m_min_value_real);
        return;
    }

    disconnect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
    m_spin->setValue(value_real);
    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));

}

void EnrichedDoubleSpinBox::applyPrecision()
{
    double increment = qPow(10, -1 * precision());

    m_spin->setSingleStep(increment);
    m_spin->setDecimals(precision());
}

void EnrichedDoubleSpinBox::restoreDefaultValue()
{
    bool ok;
    qint32 default_value_real = m_default_value.toDouble(&ok);

    if (!ok) {
        qCritical() << "Error restoring default value for double spin box";
        return;
    }

    disconnect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
    m_spin->setValue(default_value_real);
    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
