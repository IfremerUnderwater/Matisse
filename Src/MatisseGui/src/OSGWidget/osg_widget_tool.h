#ifndef OSG_WIDGET_TOOL_H
#define OSG_WIDGET_TOOL_H

#include <QObject>

#include "OSGWidget/point3d.h"

class OSGWidget;

class OSGWidgetTool : public QObject
{
    Q_OBJECT
public:
    enum type {
        None,
        Point,  // Point : one point clicked = end of tool
        Line,   // polyline : multiple points
        Area,   // polygon : id + closure of polyline -> polygon
        Slope,  // Slope tool
        MeasurePicker // measure picker tool
    };

    // singleton
    static OSGWidgetTool *instance() { return s_instance; }
    // must be initialized
    static void initialize(OSGWidget *_osg_widget);

    OSGWidget * getOSGWidget() const { return m_osg_widget; }

    ~OSGWidgetTool();

    void startTool(const type _type);
    void endTool();

signals:
    void signal_clicked(Point3D &_point);
    void signal_clickedXY(Point3D &_point, int x, int y);
    void signal_clickedLMouse(int x, int y);
    void signal_endTool(); // to be used to remove connections
    void signal_cancelTool();
    void signal_removeLastPointTool();


public slots:
    void slot_mouseButtonDown(Qt::MouseButton _button, int _x, int _y);
    void slot_cancelTool();
    void slot_removeLastPointTool();

private:
    // singleton
    OSGWidgetTool();

    static OSGWidgetTool *s_instance;

    OSGWidget *m_osg_widget;
    type m_current_type;
};

#endif // OSG_WIDGET_TOOL_H
