#include "enriched_check_box.h"

namespace matisse {

EnrichedCheckBox::EnrichedCheckBox(QWidget *_parent, QString _label, bool _checked) :
    EnrichedFormWidget(_parent)
{
    m_check = new QCheckBox(this);
    m_default_value = QString("%1").arg(_checked);
    m_check->setChecked(_checked);

    setWidget(_label, m_check);

    connect(m_check, SIGNAL(stateChanged(int)), this, SLOT(sl_valueChanged()));

}

//bool EnrichedCheckBox::currentValueChanged()
//{
//    return (_check->checkState() == Qt::Checked) != bool(_defaultValue.toInt());
//}

QString EnrichedCheckBox::currentValue()
{
    QString ret = "false";
    if (m_check->checkState() == Qt::Checked) {
        ret = "true";
    }

    return ret;
}

void EnrichedCheckBox::restoreDefaultValue()
{

    bool checked = QVariant(m_default_value).toBool();

    disconnect(m_check, SIGNAL(stateChanged(int)), this, SLOT(sl_valueChanged()));
    m_check->setChecked(checked);
    connect(m_check, SIGNAL(stateChanged(int)), this, SLOT(sl_valueChanged()));
}

void EnrichedCheckBox::applyValue(QString _new_value)
{
    bool checked = QVariant(_new_value).toBool();
    qDebug() << QString("Converted '%1' as '%2' for checkbox assignment").arg(_new_value).arg(checked);

    disconnect(m_check, SIGNAL(stateChanged(int)), this, SLOT(sl_valueChanged()));
    m_check->setChecked(checked);
    connect(m_check, SIGNAL(stateChanged(int)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
