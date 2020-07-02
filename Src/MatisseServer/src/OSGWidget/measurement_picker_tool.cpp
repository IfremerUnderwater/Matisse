#include "measurement_picker_tool.h"
#include "OSGWidget/osg_widget.h"
#include "OSGWidget/osg_widget_tool.h"

MeasurementPickerTool::MeasurementPickerTool(QObject *parent) : QObject(parent)
{
    start();
}

void MeasurementPickerTool::start()
{
    OSGWidgetTool *tool = OSGWidgetTool::instance();
    tool->slot_cancelTool();

    // start tool
    connect(tool, SIGNAL(signal_clickedLMouse( int, int)), this, SLOT(slot_toolClicked( int, int)));
    connect(tool, SIGNAL(signal_endTool()), this, SLOT(slot_toolEnded()));

    tool->startTool(OSGWidgetTool::MeasurePicker);
}

void MeasurementPickerTool::slot_toolEnded()
{
    disconnect(OSGWidgetTool::instance(), 0, this, 0);
    disconnect(OSGWidgetTool::instance()->getOSGWidget(), 0, this, 0);

    OSGWidgetTool::instance()->endTool();

    delete this;
}

void MeasurementPickerTool::slot_toolClicked(int _x, int _y)
{
    OSGWidget* w = OSGWidgetTool::instance()->getOSGWidget();

    bool exist;
    osg::ref_ptr<osg::Node> node;
    w->getIntersectionPointNode(_x,_y, node, exist);
    if(exist)
    {
        emit signal_nodeClicked(node.get());
    }
    else
    {
        emit signal_noNodeClicked();
    }
}
