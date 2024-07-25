#ifndef CAMERALIST_H
#define CAMERALIST_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include "enriched_form_widget.h"
#include "camera_info.h"
#include "camera_path_widget.h"

namespace matisse {

    class EnrichedCameraList : public EnrichedFormWidget
    {
        Q_OBJECT  // This macro must be included in any class that defines signals, slots, or uses the meta-object system.

    public:
        explicit EnrichedCameraList(QWidget* _parent = nullptr, QString _label = "", QString _default_value = "");

        bool currentValueChanged();
        virtual QString currentValue();
        virtual void restoreDefaultValue();
        void addCameraWidget(QString _cam_value = "");

    protected:
        virtual void applyValue(QString _new_value);

    private slots:
        void addDefaultCameraWidget();
        void sl_removeCameraWidget();

    private:
        void removeCameraWidget();
        QWidget* m_container_widget;
        QList<CameraWidget*> cameraWidgetList();
        QStringList camListStringToListOfCamString(QString _cam_list_string);
        QVBoxLayout* m_main_layout;
        QPushButton* m_add_button;
        QPushButton* m_remove_button;

        QString m_cam_list_default_value;
        QString m_cam_default_value;

        int m_cam_nb;

    };

}

#endif // CAMERALIST_H
