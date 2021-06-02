#include "enriched_double_spin_box.h"
#include "graphical_charter.h"

namespace matisse {

EnrichedDoubleSpinBox::EnrichedDoubleSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue):
    EnrichedDecimalValueWidget(parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();
    _spin = new QDoubleSpinBox(this);
    _spin->setFixedWidth(graph_chart.dpiScaled(PARAM_SPINBOX_WIDTH));
    minValue = minValue.trimmed().toLower();
    maxValue = maxValue.trimmed().toLower();
    QString specialValue;

    quint8 defaultPrecision = PRECISION_DEFAULT;
    double increment = qPow(10, -1 * defaultPrecision);

    bool ok;

    if (minValue.startsWith("-inf")) {
        specialValue = "-inf";
        _minValueReal = MIN_REAL;
    } else {
        _minValueReal = minValue.toDouble();
    }

    if (maxValue.startsWith("inf")) {
        specialValue = "inf";
        _maxValueReal = MAX_REAL;
        if (qAbs(_minValueReal - MIN_REAL) > increment) {
            _minValueReal -= increment;
        }
    } else {
        _maxValueReal = maxValue.toInt();
    }


    _spin->setRange(_minValueReal, _maxValueReal);
    _spin->setWrapping(true);
    _spin->setSingleStep(increment);
    _spin->setDecimals(defaultPrecision);

    if (!specialValue.isEmpty()) {
        _spin->setSpecialValueText(specialValue);
    }
    _defaultValue = defaultValue;
    double defaultValueReal = defaultValue.toDouble(&ok);
    if (!ok) {
        qWarning() << QString("Could not convert default value '%1' to real, using min value as default").arg(_defaultValue);

        _spin->setValue(_minValueReal);
        _defaultValue = QString::number(_minValueReal);
    } else {
        _spin->setValue(defaultValueReal);
    }

    setWidget(label, _spin);

    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));

}

//bool EnrichedDoubleSpinBox::currentValueChanged()
//{
//    return _spin->text() != _defaultValue;
//}

QString EnrichedDoubleSpinBox::currentValue()
{
    return _spin->text();
}

void EnrichedDoubleSpinBox::applyValue(QString newValue)
{
    bool ok;

    double valueReal = newValue.toDouble(&ok);
    if (!ok) {
        qWarning() << QString("Error converting '%1' to double for double spin box value assignment, skipping...").arg(newValue);
        return;
    }

    if (valueReal > _maxValueReal) {
        qWarning() << QString("Value '%1' greater than max value '%2', skipping...").arg(valueReal).arg(_maxValueReal);
        return;
    }

    if (valueReal < _minValueReal) {
        qWarning() << QString("Value '%1' lower than min value '%2', skipping...").arg(valueReal).arg(_minValueReal);
        return;
    }

    disconnect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
    _spin->setValue(valueReal);
    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));

}

void EnrichedDoubleSpinBox::applyPrecision()
{
    double increment = qPow(10, -1 * precision());

    _spin->setSingleStep(increment);
    _spin->setDecimals(precision());
}

void EnrichedDoubleSpinBox::restoreDefaultValue()
{
    bool ok;
    qint32 defaultValueReal = _defaultValue.toDouble(&ok);

    if (!ok) {
        qCritical() << "Error restoring default value for double spin box";
        return;
    }

    disconnect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
    _spin->setValue(defaultValueReal);
    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
}

} // namespace matisse
