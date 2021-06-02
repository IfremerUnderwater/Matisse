#ifndef MATISSE_ENRICHED_CAM_COMBO_BOX_H_
#define MATISSE_ENRICHED_CAM_COMBO_BOX_H_

#include <QComboBox>
#include <QStringList>
#include <QtDebug>

#include "enriched_form_widget.h"
#include "camera_info.h"

namespace matisse {

class EnrichedCamComboBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedCamComboBox(QWidget *parent = 0, QString label = "", QString defaultValue = "");

    bool currentValueChanged();
    virtual QString currentValue();
    qint32 currentIndex();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

private slots:
    void slot_refreshCameraList();

private:
        QComboBox m_combo;
        qint32 m_default_index;
        qint32 m_initial_index;

        CameraInfo m_cam_info;

};

} // namespace matisse

#endif // MATISSE_ENRICHED_CAM_COMBO_BOX_H_
