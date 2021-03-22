#ifndef ENRICHEDCAMCOMBOBOX_H
#define ENRICHEDCAMCOMBOBOX_H

#include <QComboBox>
#include <QStringList>
#include <QtDebug>

#include "EnrichedFormWidget.h"
#include "camera_info.h"

namespace MatisseTools {

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

        MatisseCommon::CameraInfo m_cam_info;

};
}
#endif // ENRICHEDCAMCOMBOBOX_H
