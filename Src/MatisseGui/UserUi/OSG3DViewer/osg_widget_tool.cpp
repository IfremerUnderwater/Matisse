#include "osg_widget.h"
#include "osg_widget_tool.h"

namespace matisse {

OSGWidgetTool *OSGWidgetTool::s_instance = 0;

OSGWidgetTool::OSGWidgetTool() : m_current_type(OSGWidgetTool::NONE)
{
}

OSGWidgetTool::~OSGWidgetTool()
{
}

// must be initialized
void OSGWidgetTool::initialize(OSGWidget *_osg_widget)
{
    s_instance = new OSGWidgetTool();
    s_instance->m_osg_widget = _osg_widget;
    s_instance->m_current_type = NONE;

    connect(s_instance->m_osg_widget,SIGNAL(si_onMousePress(Qt::MouseButton,int,int)),
            s_instance,SLOT(sl_mouseButtonDown(Qt::MouseButton,int,int)));
}


void OSGWidgetTool::sl_mouseButtonDown(Qt::MouseButton _button, int _x, int _y)
{
    if(m_current_type == NONE)
        return;

    if(_button == Qt::MouseButton::RightButton)
    {
        // cancel or end
        endTool();
    }

    if(_button == Qt::MouseButton::LeftButton)
    {
        if(m_current_type == MEASURE_PICKER)
        {
            emit si_clickedLMouse(_x, _y);

            return;
        }

        // clic
        bool exists = false;
        osg::Vec3d vect;
        m_osg_widget->getIntersectionPoint(_x, _y, vect, exists);
        if(exists)
        {
            Point3D point;
            point.x = vect[0];
            point.y = vect[1];
            point.z = vect[2];
            emit si_clicked(point);
            emit si_clickedXY(point, _x, _y);
        }
    }
}

void OSGWidgetTool::startTool(const eType _type)
{
    endTool();

    m_current_type = _type;

    QString msg = tr("Start tool ");
    switch(_type)
    {
    case POINT:
        msg += "Point";
        break;
    case LINE:
        msg += "Line";
        break;
    case AREA:
        msg += "Area";
        break;
    case SLOPE:
        msg += "Slope";
        break;
    case MEASURE_PICKER:
        msg += "Measure Picker";
        break;
    default:
        break;
    }

    m_osg_widget->startTool(msg);
    m_osg_widget->setCursor(Qt::CrossCursor);
}

void OSGWidgetTool::endTool()
{
    if(m_current_type != NONE)
    {
        if(!m_osg_widget->isValid())
            return;

        m_osg_widget->setCursor(Qt::ArrowCursor);

        // cancel or end
        QString msg = tr("End tool ");
        switch(m_current_type)
        {
        case POINT:
            msg += "Point";
            break;
        case LINE:
            msg += "Line";
            break;
        case AREA:
            msg += "Area";
            break;
        case SLOPE:
            msg += "Slope";
            break;
        case MEASURE_PICKER:
            msg += "Measure Picker";
            break;
        default:
            break;
        }
        m_osg_widget->endTool(msg);

        emit si_endTool();
        m_current_type = NONE;
    }
}


void OSGWidgetTool::sl_cancelTool()
{
    if(m_current_type != NONE)
    {
        emit si_cancelTool();

        endTool();
    }
}

void OSGWidgetTool::sl_removeLastPointTool()
{
    if(m_current_type != NONE)
    {
        emit si_removeLastPointTool();
    }
}

} // namespace matisse

