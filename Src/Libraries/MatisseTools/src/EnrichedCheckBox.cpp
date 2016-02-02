#include "EnrichedCheckBox.h"

using namespace MatisseTools;

EnrichedCheckBox::EnrichedCheckBox(QWidget *parent, QString label, bool checked) :
    EnrichedFormWidget(parent)
{
    _check = new QCheckBox(this);
    _defaultValue = QString("%1").arg(checked);
    _check->setChecked(checked);

    setWidget(label, _check);

    connect(_check, SIGNAL(stateChanged(int)), this, SLOT(slot_valueChanged()));

}

bool EnrichedCheckBox::currentValueChanged()
{
    return (_check->checkState() == Qt::Checked) != bool(_defaultValue.toInt());
}

QString EnrichedCheckBox::currentValue()
{
    QString ret = "false";
    if (_check->checkState() == Qt::Checked) {
        ret = "true";
    }

    return ret;
}

void EnrichedCheckBox::restoreDefaultValue()
{
    bool checked = QVariant(_defaultValue).toBool();

    _check->setChecked(checked);
}

void EnrichedCheckBox::setValue(QString newValue)
{
    bool checked = QVariant(newValue).toBool();
    qDebug() << QString("Converted '%1' as '%2' for checkbox assignment").arg(newValue).arg(checked);

    _check->setChecked(checked);
}
