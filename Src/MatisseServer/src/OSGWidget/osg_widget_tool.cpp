#include "OSGWidget/osg_widget.h"
#include "osg_widget_tool.h"

OSGWidgetTool *OSGWidgetTool::s_instance = 0;

OSGWidgetTool::OSGWidgetTool() : m_current_type(OSGWidgetTool::None)
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
    s_instance->m_current_type = None;

    connect(s_instance->m_osg_widget,SIGNAL(signal_onMousePress(Qt::MouseButton,int,int)),
            s_instance,SLOT(slot_mouseButtonDown(Qt::MouseButton,int,int)));
}


void OSGWidgetTool::slot_mouseButtonDown(Qt::MouseButton _button, int _x, int _y)
{
    if(m_current_type == None)
        return;

    if(_button == Qt::MouseButton::RightButton)
    {
        // cancel or end
        endTool();
    }

//    if(_button == Qt::MouseButton::MiddleButton)
//    {
//          slot_removeLastPointTool();
//    }

    if(_button == Qt::MouseButton::LeftButton)
    {
        if(m_current_type == MeasurePicker)
        {
            emit signal_clickedLMouse(_x, _y);

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
            emit signal_clicked(point);
            emit signal_clickedXY(point, _x, _y);
        }
    }
}

void OSGWidgetTool::startTool(const type _type)
{
    endTool();

    m_current_type = _type;

    QString msg = tr("Start tool ");
    switch(_type)
    {
    case Point:
        msg += "Point";
        break;
    case Line:
        msg += "Line";
        break;
    case Area:
        msg += "Area";
        break;
    case Slope:
        msg += "Slope";
        break;
    case MeasurePicker:
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
    if(m_current_type != None)
    {
        if(!m_osg_widget->isValid())
            return;

        m_osg_widget->setCursor(Qt::ArrowCursor);

        // cancel or end
        QString msg = tr("End tool ");
        switch(m_current_type)
        {
        case Point:
            msg += "Point";
            break;
        case Line:
            msg += "Line";
            break;
        case Area:
            msg += "Area";
            break;
        case Slope:
            msg += "Slope";
            break;
        case MeasurePicker:
            msg += "Measure Picker";
            break;
        default:
            break;
        }
        m_osg_widget->endTool(msg);

        emit signal_endTool();
        m_current_type = None;
    }
}


void OSGWidgetTool::slot_cancelTool()
{
    if(m_current_type != None)
    {
        emit signal_cancelTool();

        endTool();
    }
}

void OSGWidgetTool::slot_removeLastPointTool()
{
    if(m_current_type != None)
    {
        emit signal_removeLastPointTool();
    }
}

