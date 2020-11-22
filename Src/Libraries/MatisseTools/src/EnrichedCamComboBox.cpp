#include "EnrichedCamComboBox.h"
#include "camera_manager.h"

using namespace MatisseTools;
using namespace MatisseCommon;

EnrichedCamComboBox::EnrichedCamComboBox(QWidget *parent, QString label, QString defaultValue):
    EnrichedFormWidget(parent)
{

    QStringList values = CameraManager::instance().cameraList();

    _combo = new QComboBox(this);
    _combo->setEditable(false);
    _combo->addItems(values);
    _defaultValue = defaultValue;
    _defaultIndex = values.indexOf(_defaultValue);
    _combo->setCurrentIndex(_defaultIndex);
    setWidget(label, _combo);

    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));

}

bool EnrichedCamComboBox::currentValueChanged()
{
    return (_combo->currentIndex() != _initialIndex);
}

QString EnrichedCamComboBox::currentValue()
{
    return _combo->currentText();
}

qint32 EnrichedCamComboBox::currentIndex()
{
    return _combo->currentIndex();
}

void EnrichedCamComboBox::applyValue(QString newValue)
{
    int index = _combo->findText(newValue, Qt::MatchExactly);

    if (index == -1) {
        qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(newValue);
        return;
    }

    disconnect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    _combo->setCurrentIndex(index);
    _initialIndex = index;
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}

void EnrichedCamComboBox::restoreDefaultValue()
{
    disconnect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    _combo->setCurrentIndex(_defaultIndex);
    connect(_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}
