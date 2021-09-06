#include "measurement_picker_tool.h"
#include "osg_widget.h"
#include "osg_widget_tool.h"

namespace matisse {

MeasurementPickerTool::MeasurementPickerTool(QObject *_parent) : QObject(_parent)
{
    sl_start();
}

void MeasurementPickerTool::sl_start()
{
    OSGWidgetTool *tool = OSGWidgetTool::instance();
    tool->sl_cancelTool();

    // start tool
    connect(tool, SIGNAL(si_clickedLMouse( int, int)), this, SLOT(sl_toolClicked( int, int)));
    connect(tool, SIGNAL(si_endTool()), this, SLOT(sl_toolEnded()));

    tool->startTool(OSGWidgetTool::MEASURE_PICKER);
}

void MeasurementPickerTool::sl_toolEnded()
{
    disconnect(OSGWidgetTool::instance(), 0, this, 0);
    disconnect(OSGWidgetTool::instance()->getOSGWidget(), 0, this, 0);

    OSGWidgetTool::instance()->endTool();

    delete this;
}

void MeasurementPickerTool::sl_toolClicked(int _x, int _y)
{
    OSGWidget* w = OSGWidgetTool::instance()->getOSGWidget();

    bool exist;
    osg::ref_ptr<osg::Node> node;
    w->getIntersectionPointNode(_x,_y, node, exist);
    if(exist)
    {
        emit si_nodeClicked(node.get());
    }
    else
    {
        emit si_noNodeClicked();
    }
}

} // namespace matisse
