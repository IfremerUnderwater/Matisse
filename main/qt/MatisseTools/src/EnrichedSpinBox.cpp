#include <QDebug>

#include "EnrichedSpinBox.h"

using namespace MatisseTools;

EnrichedSpinBox::EnrichedSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue):
    EnrichedFormWidget(parent)
{
    _spin = new QSpinBox(this);
    minValue = minValue.trimmed().toLower();
    maxValue = maxValue.trimmed().toLower();
    QString specialValue;
    qint32 minValueInt;
    qint32 maxValueInt;

    bool ok;

    _defaultValue = defaultValue;

    if (minValue.startsWith("-inf")) {
        specialValue = "-inf";
        minValueInt = MIN_SINT32;
    } else {
        minValueInt = minValue.toInt();
    }

    if (maxValue.startsWith("inf")) {
        specialValue = "inf";
        maxValueInt = MAX_SINT32;
        if (minValueInt > (qint32)MIN_SINT32) {
            minValueInt--;
        }
    } else {
        maxValueInt = maxValue.toInt();
    }

    if (!specialValue.isEmpty()) {
        _spin->setSpecialValueText(specialValue);
    }
    _spin->setRange(minValueInt, maxValueInt);
    _spin->setWrapping(true);

    qint32 defaultValueInt = defaultValue.toInt(&ok);
    if (!ok) {
        _spin->setValue(minValueInt);
    } else {
        _spin->setValue(defaultValueInt);
    }

    setWidget(label, _spin);

    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));

}

bool EnrichedSpinBox::currentValueChanged()
{
    return _spin->text() != _defaultValue;
}

QString EnrichedSpinBox::currentValue()
{
    return _spin->text();
}
