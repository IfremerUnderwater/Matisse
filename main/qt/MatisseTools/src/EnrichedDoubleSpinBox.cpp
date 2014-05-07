#include "EnrichedDoubleSpinBox.h"

using namespace MatisseTools;

EnrichedDoubleSpinBox::EnrichedDoubleSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _spin = new QDoubleSpinBox(this);
    minValue = minValue.trimmed().toLower();
    maxValue = maxValue.trimmed().toLower();
    QString specialValue;
    qreal minValueReal;
    qreal maxValueReal;

    qreal epsilon = 0.001;

    bool ok;

    if (minValue.startsWith("-inf")) {
        specialValue = "-inf";
        minValueReal = MIN_REAL;
    } else {
        minValueReal = minValue.toDouble();
    }

    if (maxValue.startsWith("inf")) {
        specialValue = "inf";
        maxValueReal = MAX_REAL;
        if (qAbs(minValueReal - MIN_REAL) > epsilon) {
            minValueReal -= epsilon;
        }
    } else {
        maxValueReal = maxValue.toInt();
    }


    _spin->setRange(minValueReal, maxValueReal);
    _spin->setWrapping(true);
    _spin->setSingleStep(epsilon);
    _spin->setDecimals(3);

    if (!specialValue.isEmpty()) {
        _spin->setSpecialValueText(specialValue);
    }
    _defaultValue = defaultValue;
    qreal defaultValueReal = defaultValue.toDouble(&ok);
    if (!ok) {
        _spin->setValue(minValueReal);
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
