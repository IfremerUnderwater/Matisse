#include "camera_path_widget.h"
#include <QFileDialog>
#include <QtDebug>
#include "camera_manager.h"

namespace matisse {

    CameraWidget::CameraWidget(QWidget* parent, QString _default_value) : QWidget(parent)
    {
        // Construct widget ***********************************************************************
        // Create combo
        m_camera_combo_box = new QComboBox(this);

        // Create img path widgets
        m_img_path_le = new QLineEdit(this);
        m_select_img_path_btn = new QPushButton("Img", this);
        m_select_img_path_btn->setMaximumWidth(40);

        // Create nav path widgets
        m_nav_path_le = new QLineEdit(this);
        m_select_nav_path_btn = new QPushButton("Nav", this);
        m_select_nav_path_btn->setMaximumWidth(40);

        connect(m_select_img_path_btn, &QPushButton::clicked, this, &CameraWidget::selectImgPath);
        connect(m_select_nav_path_btn, &QPushButton::clicked, this, &CameraWidget::selectNavPath);

        // Vertical layout containing combo box, image path and nav path
        QVBoxLayout* v_layout = new QVBoxLayout(this);
        v_layout->addWidget(m_camera_combo_box);

        // Image path layout
        QHBoxLayout* img_path_layout = new QHBoxLayout();
        v_layout->addLayout(img_path_layout);
        img_path_layout->addWidget(m_img_path_le, 1);
        img_path_layout->addWidget(m_select_img_path_btn, 0);

        // Image path layout
        QHBoxLayout* nav_path_layout = new QHBoxLayout();
        v_layout->addLayout(nav_path_layout);
        nav_path_layout->addWidget(m_nav_path_le, 1);
        nav_path_layout->addWidget(m_select_nav_path_btn, 0);

        setLayout(v_layout);
        // Construct widget ***********************************************************************

        // Configure behaviors ********************************************************************
        // serialized default value
        m_default_value = _default_value;
        QStringList default_split = m_default_value.split("|");
        m_cam_default_value = default_split[0];
        m_img_path_default_value=default_split[1];
        m_nav_path_default_value=default_split[2];
        m_img_path_le->setText(m_img_path_default_value);
        m_nav_path_le->setText(m_nav_path_default_value);

        QStringList values = CameraManager::instance().cameraList();
        m_cam_info.fromQString(m_cam_default_value);

        m_camera_combo_box->setEditable(false);
        m_camera_combo_box->addItems(values);

        // add default value if missing
        QStringList items_in_combo_box;
        for (int index = 0; index < m_camera_combo_box->count(); index++)
            items_in_combo_box << m_camera_combo_box->itemText(index);

        if (!items_in_combo_box.contains(m_cam_info.cameraName()))
            m_camera_combo_box->addItem(m_cam_info.cameraName());

        m_default_index = m_camera_combo_box->findText(m_cam_info.cameraName(), Qt::MatchExactly);
        m_camera_combo_box->setCurrentIndex(m_default_index);

        connect(m_camera_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
        connect(&CameraManager::instance(), SIGNAL(si_cameraListChanged()), this, SLOT(sl_refreshCameraList()));

    }


    void CameraWidget::selectImgPath()
    {
        QString filePath = QFileDialog::getExistingDirectory(this, tr("Select Image Folder"));
        if (!filePath.isEmpty())
        {
            m_img_path_le->setText(filePath);
            emit si_valueChanged();
        }
    }

    void CameraWidget::selectNavPath()
    {
        QString filePath = QFileDialog::getOpenFileName(this, tr("Select Navigation file"));
        if (!filePath.isEmpty())
        {
            m_nav_path_le->setText(filePath);
            emit si_valueChanged();
        }
    }

    bool CameraWidget::currentValueChanged()
    {
        return (m_camera_combo_box->currentIndex() != m_initial_index)
            || (m_img_path_le->text()!= m_img_path_default_value)
            || (m_nav_path_le->text() != m_nav_path_default_value);
    }

    QString CameraWidget::currentValue()
    {
        QString cam_as_string;

        if (m_camera_combo_box->currentText() == "Unknown")
        {
            CameraInfo unknown_cam_info;
            unknown_cam_info.setCameraName("Unknown");
            cam_as_string = unknown_cam_info.toQString();
        }
        else
        {
            cam_as_string = CameraManager::instance().cameraByName(m_camera_combo_box->currentText()).toQString();
        }

        return cam_as_string+"|"+m_img_path_le->text()+"|"+m_nav_path_le->text();
    }

    qint32 CameraWidget::currentIndex()
    {
        return m_camera_combo_box->currentIndex();
    }

    void CameraWidget::applyValue(QString _new_value)
    {
        QStringList value_split = _new_value.split("|");

        if (value_split.size() != 3)
            return;

        m_cam_info.fromQString(value_split[0]);
        m_img_path_le->setText(value_split[1]);
        m_nav_path_le->setText(value_split[2]);

        QStringList items_in_combo_box;
        for (int index = 0; index < m_camera_combo_box->count(); index++)
            items_in_combo_box << m_camera_combo_box->itemText(index);

        if (!items_in_combo_box.contains(m_cam_info.cameraName()))
            m_camera_combo_box->addItem(m_cam_info.cameraName());

        int index = m_camera_combo_box->findText(m_cam_info.cameraName(), Qt::MatchExactly);

        if (index == -1) {
            qWarning() << QString("Could not assign value '%1' : not found in combo box").arg(_new_value);
            return;
        }

        disconnect(m_camera_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
        m_camera_combo_box->setCurrentIndex(index);
        m_initial_index = index;
        connect(m_camera_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    }

    void CameraWidget::sl_refreshCameraList()
    {
        QString backup_current_value = currentValue();

        // add database cam
        QStringList values = CameraManager::instance().cameraList();
        m_camera_combo_box->clear();
        m_camera_combo_box->addItems(values);

        // add unknown option
        m_camera_combo_box->addItem("Unknown");

        // apply back the current value
        applyValue(backup_current_value);
    }

    void CameraWidget::restoreDefaultValue()
    {
        m_img_path_le->setText(m_img_path_default_value);
        m_nav_path_le->setText(m_nav_path_default_value);

        disconnect(m_camera_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
        m_camera_combo_box->setCurrentIndex(m_default_index);
        m_cam_info.fromQString(m_cam_default_value);
        connect(m_camera_combo_box, SIGNAL(currentIndexChanged(int)), this, SLOT(sl_valueChanged()));
    }

    void CameraWidget::sl_valueChanged()
    {
        emit si_valueChanged();
    }

}