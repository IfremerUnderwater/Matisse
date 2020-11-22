#include "EnrichedCamComboBox.h"
#include "camera_manager.h"

using namespace MatisseTools;
using namespace MatisseCommon;

EnrichedCamComboBox::EnrichedCamComboBox(QWidget *parent, QString label, QString defaultValue):
    EnrichedFormWidget(parent),
    m_combo(this)
{

    QStringList values = CameraManager::instance().cameraList();
    m_cam_info.fromQString(defaultValue);

    m_combo.setEditable(false);
    m_combo.addItems(values);

    // add default value if missing
    QStringList items_in_combo_box;
    for (int index = 0; index < m_combo.count(); index++)
        items_in_combo_box << m_combo.itemText(index);

    if( !items_in_combo_box.contains(m_cam_info.cameraName()))
        m_combo.addItem(m_cam_info.cameraName());

    _defaultValue = defaultValue;
    m_default_index = m_combo.findText(m_cam_info.cameraName(), Qt::MatchExactly);
    m_combo.setCurrentIndex(m_default_index);

    setWidget(label, &m_combo);

    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));

}

bool EnrichedCamComboBox::currentValueChanged()
{
    return (m_combo.currentIndex() != m_initial_index);
}

QString EnrichedCamComboBox::currentValue()
{
    return m_cam_info.toQString();
}

qint32 EnrichedCamComboBox::currentIndex()
{
    return m_combo.currentIndex();
}

void EnrichedCamComboBox::applyValue(QString newValue)
{
    m_cam_info.fromQString(newValue);

    QStringList items_in_combo_box;
    for (int index = 0; index < m_combo.count(); index++)
        items_in_combo_box << m_combo.itemText(index);

    if( !items_in_combo_box.contains(m_cam_info.cameraName()))
        m_combo.addItem(m_cam_info.cameraName());

    int index = m_combo.findText(m_cam_info.cameraName(), Qt::MatchExactly);

    if (index == -1) {
        qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(newValue);
        return;
    }

    disconnect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    m_combo.setCurrentIndex(index);
    m_initial_index = index;
    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}

void EnrichedCamComboBox::restoreDefaultValue()
{
    disconnect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
    m_combo.setCurrentIndex(m_default_index);
    m_cam_info.fromQString(_defaultValue);
    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_valueChanged()));
}
