#include <QDebug>
#include "graphical_charter.h"
#include "enriched_spin_box.h"

namespace matisse {

EnrichedSpinBox::EnrichedSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue):
    EnrichedFormWidget(parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    _spin = new QSpinBox(this);
    _spin->setFixedWidth(graph_chart.dpiScaled(PARAM_SPINBOX_WIDTH));
    minValue = minValue.trimmed().toLower();
    maxValue = maxValue.trimmed().toLower();
    QString specialValue;

    bool ok;

    _defaultValue = defaultValue;

    if (minValue.startsWith("-inf")) {
        specialValue = "-inf";
        _minValueInt = MIN_SINT32;
    } else {
        _minValueInt = minValue.toInt();
    }

    if (maxValue.startsWith("inf")) {
        specialValue = "inf";
        _maxValueInt = MAX_SINT32;
        if (_minValueInt > (qint32)MIN_SINT32) {
            _minValueInt--;
        }
    } else {
        _maxValueInt = maxValue.toInt();
    }

    // TODO Il est possible que l'affichage ne marche pas pour un intervale ]-inf;inf[
    if (!specialValue.isEmpty()) {
        _spin->setSpecialValueText(specialValue);
    }
    _spin->setRange(_minValueInt, _maxValueInt);
    _spin->setWrapping(true);


    QString minValueStr = QString::number(_minValueInt);
    QString maxValueStr = QString::number(_maxValueInt);

    if (defaultValue.startsWith("-inf")) {

        if (_minValueInt > MIN_SINT32) {
            qWarning() << QString("Default value '-inf' out of range : min value=%1").arg(_minValueInt);
        }

        _defaultValue = minValueStr;
        _spin->setValue(_minValueInt);

    } else if (defaultValue.startsWith("inf")) {

        if (_maxValueInt < MAX_SINT32) {
            qWarning() << QString("Default value 'inf' out of range : max value=%1").arg(_maxValueInt);
            _defaultValue = maxValueStr;
            _spin->setValue(_maxValueInt);
        } else {
            _defaultValue = minValueStr;
            _spin->setValue(_minValueInt); // pour afficher le 'specialValueText'
        }

    } else { // valeur entiere

        qint32 defaultValueInt = defaultValue.toInt(&ok);

        if (!ok) {
            qWarning() << QString("Could not convert default value '%1' to int, using min value as default").arg(_defaultValue);

            _spin->setValue(_minValueInt);
            _defaultValue = minValueStr;
        } else {
            _spin->setValue(defaultValueInt);
        }
    }

    setWidget(label, _spin);

    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));

}

//bool EnrichedSpinBox::currentValueChanged()
//{
//    return _spin->text() != _defaultValue;
//}

QString EnrichedSpinBox::currentValue()
{
    return _spin->text();
}

void EnrichedSpinBox::applyValue(QString newValue)
{
    disconnect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));

    if (newValue.startsWith("-inf")) {

        if (_minValueInt > MIN_SINT32) {
            qWarning() << QString("New value '-inf' out of range : min value=%1").arg(_minValueInt);
        }

        _spin->setValue(_minValueInt);

    } else if (newValue.startsWith("inf")) {

        if (_maxValueInt < MAX_SINT32) {
            qWarning() << QString("New value 'inf' out of range : max value=%1").arg(_maxValueInt);
            _spin->setValue(_maxValueInt);
        } else {
            _spin->setValue(_minValueInt);
        }

    } else {

        bool ok;
        qint32 valueInt = newValue.toInt(&ok);

        if (!ok) {
            qCritical() << QString("Error converting '%1' to int for spin box value assignment, skipping...").arg(newValue);
            return;
        }

        if (valueInt > _maxValueInt) {
            qWarning() << QString("Value '%1' greater than max value '%2', skipping...").arg(valueInt).arg(_maxValueInt);
            return;
        }

        if (valueInt < _minValueInt) {
            qWarning() << QString("Value '%1' lower than min value '%2', skipping...").arg(valueInt).arg(_minValueInt);
            return;
        }

        _spin->setValue(valueInt);
    }

    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
}

void EnrichedSpinBox::restoreDefaultValue()
{
    bool ok;
    qint32 defaultValueInt = _defaultValue.toInt(&ok);

    if (!ok) {
        qCritical() << "Error restoring default value for spin box";
        return;
    }

    disconnect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
    _spin->setValue(defaultValueInt);
    connect(_spin, SIGNAL(valueChanged(QString)), this, SLOT(slot_valueChanged()));
}

} // namespace matisse
