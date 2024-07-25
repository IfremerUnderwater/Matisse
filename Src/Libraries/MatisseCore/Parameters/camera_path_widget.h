#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include "camera_info.h"

namespace matisse {

    class CameraWidget : public QWidget
    {
        Q_OBJECT  // This macro must be included in any class that defines signals, slots, or uses the meta-object system.

    public:
        explicit CameraWidget(QWidget* parent, QString _default_value);

        bool currentValueChanged();
        virtual QString currentValue();
        qint32 currentIndex();
        virtual void restoreDefaultValue();

    protected:
        virtual void applyValue(QString _new_value);

    private slots:
        void selectImgPath();  // Slot to handle path selection
        void selectNavPath();  // Slot to handle path selection
        void sl_refreshCameraList();

    private:
        QComboBox* m_camera_combo_box;
        QLineEdit* m_img_path_le;
        QPushButton* m_select_img_path_btn;
        QLineEdit* m_nav_path_le;
        QPushButton* m_select_nav_path_btn;

        qint32 m_default_index;
        qint32 m_initial_index;
        QString m_default_value;
        QString m_cam_default_value;
        QString m_img_path_default_value;
        QString m_nav_path_default_value;

        CameraInfo m_cam_info;
    };

}

#endif // CAMERAWIDGET_H
