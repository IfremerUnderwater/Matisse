#include <QDebug>
#include "graphical_charter.h"
#include "enriched_spin_box.h"

namespace matisse {

EnrichedSpinBox::EnrichedSpinBox(QWidget *_parent, QString _label, QString _min_value, QString _max_value, QString _default_value):
    EnrichedFormWidget(_parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    m_spin = new QSpinBox(this);
    m_spin->setFixedWidth(graph_chart.dpiScaled(PARAM_SPINBOX_WIDTH));
    _min_value = _min_value.trimmed().toLower();
    _max_value = _max_value.trimmed().toLower();
    QString specialValue;

    bool ok;

    m_default_value = _default_value;

    if (_min_value.startsWith("-inf")) {
        specialValue = "-inf";
        m_min_value_int = MIN_SINT32;
    } else {
        m_min_value_int = _min_value.toInt();
    }

    if (_max_value.startsWith("inf")) {
        specialValue = "inf";
        m_max_value_int = MAX_SINT32;
        if (m_min_value_int > (qint32)MIN_SINT32) {
            m_min_value_int--;
        }
    } else {
        m_max_value_int = _max_value.toInt();
    }

    // TODO Il est possible que l'affichage ne marche pas pour un intervale ]-inf;inf[
    if (!specialValue.isEmpty()) {
        m_spin->setSpecialValueText(specialValue);
    }
    m_spin->setRange(m_min_value_int, m_max_value_int);
    m_spin->setWrapping(true);


    QString min_value_str = QString::number(m_min_value_int);
    QString max_value_str = QString::number(m_max_value_int);

    if (_default_value.startsWith("-inf")) {

        if (m_min_value_int > MIN_SINT32) {
            qWarning() << QString("Default value '-inf' out of range : min value=%1").arg(m_min_value_int);
        }

        m_default_value = min_value_str;
        m_spin->setValue(m_min_value_int);

    } else if (_default_value.startsWith("inf")) {

        if (m_max_value_int < MAX_SINT32) {
            qWarning() << QString("Default value 'inf' out of range : max value=%1").arg(m_max_value_int);
            m_default_value = max_value_str;
            m_spin->setValue(m_max_value_int);
        } else {
            m_default_value = min_value_str;
            m_spin->setValue(m_min_value_int); // pour afficher le 'specialValueText'
        }

    } else { // valeur entiere

        qint32 default_value_int = _default_value.toInt(&ok);

        if (!ok) {
            qWarning() << QString("Could not convert default value '%1' to int, using min value as default").arg(m_default_value);

            m_spin->setValue(m_min_value_int);
            m_default_value = min_value_str;
        } else {
            m_spin->setValue(default_value_int);
        }
    }

    setWidget(_label, m_spin);

    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));

}


QString EnrichedSpinBox::currentValue()
{
    return m_spin->text();
}

void EnrichedSpinBox::applyValue(QString _new_value)
{
    disconnect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));

    if (_new_value.startsWith("-inf")) {

        if (m_min_value_int > MIN_SINT32) {
            qWarning() << QString("New value '-inf' out of range : min value=%1").arg(m_min_value_int);
        }

        m_spin->setValue(m_min_value_int);

    } else if (_new_value.startsWith("inf")) {

        if (m_max_value_int < MAX_SINT32) {
            qWarning() << QString("New value 'inf' out of range : max value=%1").arg(m_max_value_int);
            m_spin->setValue(m_max_value_int);
        } else {
            m_spin->setValue(m_min_value_int);
        }

    } else {

        bool ok;
        qint32 value_int = _new_value.toInt(&ok);

        if (!ok) {
            qCritical() << QString("Error converting '%1' to int for spin box value assignment, skipping...").arg(_new_value);
            return;
        }

        if (value_int > m_max_value_int) {
            qWarning() << QString("Value '%1' greater than max value '%2', skipping...").arg(value_int).arg(m_max_value_int);
            return;
        }

        if (value_int < m_min_value_int) {
            qWarning() << QString("Value '%1' lower than min value '%2', skipping...").arg(value_int).arg(m_min_value_int);
            return;
        }

        m_spin->setValue(value_int);
    }

    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
}

void EnrichedSpinBox::restoreDefaultValue()
{
    bool ok;
    qint32 default_value_int = m_default_value.toInt(&ok);

    if (!ok) {
        qCritical() << "Error restoring default value for spin box";
        return;
    }

    disconnect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
    m_spin->setValue(default_value_int);
    connect(m_spin, SIGNAL(valueChanged(QString)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
