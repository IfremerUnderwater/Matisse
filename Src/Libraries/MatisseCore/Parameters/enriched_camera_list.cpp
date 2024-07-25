#include "enriched_camera_list.h"
#include <QtDebug>
#include <QCoreApplication>

namespace matisse {

    EnrichedCameraList::EnrichedCameraList(QWidget* _parent, QString _label, QString _default_value) : EnrichedFormWidget(_parent), m_cam_nb(0), m_main_layout(nullptr)
    {
        // Graphical conception ***************************************************************
        m_container_widget = new QWidget(this);

        m_main_layout = new QVBoxLayout(m_container_widget);

        m_add_button = new QPushButton("+", m_container_widget);
        m_remove_button = new QPushButton("-", m_container_widget);

        connect(m_add_button, &QPushButton::clicked, this, &EnrichedCameraList::addDefaultCameraWidget);
        connect(m_remove_button, &QPushButton::clicked, this, &EnrichedCameraList::removeCameraWidget);

        QHBoxLayout* buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(m_add_button);
        buttonLayout->addWidget(m_remove_button);

        m_main_layout->addLayout(buttonLayout);

        setLayout(m_main_layout);

        setWidget(_label, m_container_widget);

        // Graphical conception ***************************************************************

        m_cam_list_default_value = _default_value;
        qDebug() << "Default camlist = " << m_cam_list_default_value;
        QStringList list_cam_strings = camListStringToListOfCamString(m_cam_list_default_value);
        m_cam_default_value= list_cam_strings[0]; // Take the first as default

        applyValue(m_cam_list_default_value);
    }

    void EnrichedCameraList::addCameraWidget(QString _cam_value)
    {
        qDebug() << "Layout size = " << m_main_layout->count();
        CameraWidget* camera_widget = new CameraWidget(m_container_widget, _cam_value);
        m_main_layout->insertWidget(0, camera_widget);

        connect(camera_widget, SIGNAL(si_valueChanged()), this, SLOT(sl_valueChanged()));
        //while(m_cam_nb!= m_main_layout->count()-1)
        //    QCoreApplication::processEvents();
    }

    void EnrichedCameraList::addDefaultCameraWidget()
    {
        addCameraWidget(m_cam_default_value);
    }

    void EnrichedCameraList::removeCameraWidget()
    {
        if (m_main_layout->count() > 1) // Ensure we don't remove the buttons
        {
            QLayoutItem* item = m_main_layout->takeAt(0);
            disconnect(item->widget(), SIGNAL(si_valueChanged()), this, SLOT(sl_valueChanged()));
            delete item->widget();
            delete item;
        }
    }

    QList<CameraWidget*> EnrichedCameraList::cameraWidgetList()
    {
        QList<CameraWidget*> cam_widget_list;

        for (int i = 0; i < m_main_layout->count() - 1; i++)
        {
            QLayoutItem* item = m_main_layout->takeAt(i);
            cam_widget_list.push_back( dynamic_cast<CameraWidget*>( item->widget() ) );
        }
        return cam_widget_list;
    }

    QStringList EnrichedCameraList::camListStringToListOfCamString(QString _cam_list_string)
    {
        QStringList params_split = _cam_list_string.split("|");
        QStringList cam_split;
        if ((params_split.size() % 3) != 0)
            return QStringList();
        for (int i = 0; i < params_split.size() / 3; i++)
        {
            QString cam_params("");
            cam_params = params_split[3 * i + 0];
            cam_params = cam_params + "|" + params_split[3 * i + 1];
            cam_params = cam_params + "|" + params_split[3 * i + 2];
            cam_split.append(cam_params);
        }
        return cam_split;
    }

    bool EnrichedCameraList::currentValueChanged()
    {
        bool changed = false;
        for (auto camera : cameraWidgetList())
        {
            changed |= camera->currentValueChanged();
        }
        return changed;
    }

    QString EnrichedCameraList::currentValue()
    {
        auto camera_list = cameraWidgetList();

		if (camera_list.size() > 0)
		{
			QString serialized_value = camera_list[0]->currentValue();
			for (int i = 1; i < camera_list.size(); i++)
			{
				serialized_value = serialized_value + "|" + camera_list[i]->currentValue();
			}
			return serialized_value;

		}
		else
		{
			return "";
		}
    }

    void EnrichedCameraList::restoreDefaultValue()
    {
        applyValue(m_cam_list_default_value);
    }

    void EnrichedCameraList::applyValue(QString _new_value)
    {
        qDebug() << "applyvalue camlist = " << _new_value;
        _new_value = "Tata; 0; 0; 0, 0, 0, 0, 0, 0, 0, 0, 0; 0; 0, 0, 0, 0, 0; 0, 0, 0, 0, 0, 0|hgfdh|hgfdh";
        while (m_main_layout->count() > 1)
        {
            removeCameraWidget();
        }

        QStringList list_cam_strings = camListStringToListOfCamString(_new_value);

        if (list_cam_strings.size() < 1)
            return;

        for (QString cam_value : list_cam_strings)
        {
            qDebug() << "addcam with value = " << cam_value;
            addCameraWidget(cam_value);
        }
    }

} // namespace matisse