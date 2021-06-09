#include "enriched_cam_combo_box.h"
#include "camera_manager.h"

namespace matisse {

EnrichedCamComboBox::EnrichedCamComboBox(QWidget *_parent, QString _label, QString _default_value):
    EnrichedFormWidget(_parent),
    m_combo(this)
{

    QStringList values = CameraManager::instance().cameraList();
    m_cam_info.fromQString(_default_value);

    m_combo.setEditable(false);
    m_combo.addItems(values);

    // add default value if missing
    QStringList items_in_combo_box;
    for (int index = 0; index < m_combo.count(); index++)
        items_in_combo_box << m_combo.itemText(index);

    if( !items_in_combo_box.contains(m_cam_info.cameraName()))
        m_combo.addItem(m_cam_info.cameraName());

    m_default_value = _default_value;
    m_default_index = m_combo.findText(m_cam_info.cameraName(), Qt::MatchExactly);
    m_combo.setCurrentIndex(m_default_index);

    setWidget(_label, &m_combo);

    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));

    connect(&CameraManager::instance(),SIGNAL(si_cameraListChanged()),this,SLOT(sl_refreshCameraList()));

}

bool EnrichedCamComboBox::currentValueChanged()
{
    return (m_combo.currentIndex() != m_initial_index);
}

QString EnrichedCamComboBox::currentValue()
{
    if (m_combo.currentText()=="Unknown")
    {
        CameraInfo unknown_cam_info;
        unknown_cam_info.setCameraName("Unknown");
        return unknown_cam_info.toQString();
    }
    else
    {
        return CameraManager::instance().cameraByName(m_combo.currentText()).toQString();
    }
}

qint32 EnrichedCamComboBox::currentIndex()
{
    return m_combo.currentIndex();
}

void EnrichedCamComboBox::applyValue(QString _new_value)
{
    m_cam_info.fromQString(_new_value);

    QStringList items_in_combo_box;
    for (int index = 0; index < m_combo.count(); index++)
        items_in_combo_box << m_combo.itemText(index);

    if( !items_in_combo_box.contains(m_cam_info.cameraName()))
        m_combo.addItem(m_cam_info.cameraName());

    int index = m_combo.findText(m_cam_info.cameraName(), Qt::MatchExactly);

    if (index == -1) {
        qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(_new_value);
        return;
    }

    disconnect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    m_combo.setCurrentIndex(index);
    m_initial_index = index;
    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
}

void EnrichedCamComboBox::sl_refreshCameraList()
{
    QString backup_current_value = currentValue();

    // add database cam
    QStringList values = CameraManager::instance().cameraList();
    m_combo.clear();
    m_combo.addItems(values);

    // add unknown option
    m_combo.addItem("Unknown");

    // apply back the current value
    applyValue(backup_current_value);
}

void EnrichedCamComboBox::restoreDefaultValue()
{
    disconnect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    m_combo.setCurrentIndex(m_default_index);
    m_cam_info.fromQString(m_default_value);
    connect(&m_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse
