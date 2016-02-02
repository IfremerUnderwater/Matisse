#include "EnrichedDoubleSpinBox.h"

using namespace MatisseTools;

EnrichedDoubleSpinBox::EnrichedDoubleSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _spin = new QDoubleSpinBox(this);
    minValue = minValue.trimmed().toLower();
    maxValue = maxValue.trimmed().toLower();
    QString specialValue;

    qreal epsilon = 0.001;

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
        if (qAbs(_minValueReal - MIN_REAL) > epsilon) {
            _minValueReal -= epsilon;
        }
    } else {
        _maxValueReal = maxValue.toInt();
    }


    _spin->setRange(_minValueReal, _maxValueReal);
    _spin->setWrapping(true);
    _spin->setSingleStep(epsilon);
    _spin->setDecimals(3);

    if (!specialValue.isEmpty()) {
        _spin->setSpecialValueText(specialValue);
    }
    _defaultValue = defaultValue;
    qreal defaultValueReal = defaultValue.toDouble(&ok);
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

bool EnrichedDoubleSpinBox::currentValueChanged()
{
    return _spin->text() != _defaultValue;
}

QString EnrichedDoubleSpinBox::currentValue()
{
    return _spin->text();
}

void EnrichedDoubleSpinBox::setValue(QString newValue)
{
    bool ok;

    qreal valueReal = newValue.toDouble(&ok);
    if (!ok) {
        qWarning() << QString("Error converting '%1' to qreal for double spin box value assignment, skipping...").arg(newValue);
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

    _spin->setValue(valueReal);

}

void EnrichedDoubleSpinBox::restoreDefaultValue()
{
    bool ok;
    qint32 defaultValueReal = _defaultValue.toDouble(&ok);

    if (!ok) {
        qCritical() << "Error restoring default value for double spin box";
        return;
    }

    _spin->setValue(defaultValueReal);
}
