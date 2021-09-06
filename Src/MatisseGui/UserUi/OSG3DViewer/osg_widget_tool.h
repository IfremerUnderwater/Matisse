#ifndef MATISSE_OSG_WIDGET_TOOL_H_
#define MATISSE_OSG_WIDGET_TOOL_H_

#include <QObject>

#include "point3d.h"

namespace matisse {

class OSGWidget;

class OSGWidgetTool : public QObject
{
    Q_OBJECT
public:
    enum eType {
        NONE,
        POINT,  // Point : one point clicked = end of tool
        LINE,   // polyline : multiple points
        AREA,   // polygon : id + closure of polyline -> polygon
        SLOPE,  // Slope tool
        MEASURE_PICKER // measure picker tool
    };

    // singleton
    static OSGWidgetTool *instance() { return s_instance; }
    // must be initialized
    static void initialize(OSGWidget *_osg_widget);

    OSGWidget * getOSGWidget() const { return m_osg_widget; }

    ~OSGWidgetTool();

    void startTool(const eType _type);
    void endTool();

signals:
    void si_clicked(Point3D &_point);
    void si_clickedXY(Point3D &_point, int x, int y);
    void si_clickedLMouse(int _x, int _y);
    void si_endTool(); // to be used to remove connections
    void si_cancelTool();
    void si_removeLastPointTool();


public slots:
    void sl_mouseButtonDown(Qt::MouseButton _button, int _x, int _y);
    void sl_cancelTool();
    void sl_removeLastPointTool();

private:
    // singleton
    OSGWidgetTool();

    static OSGWidgetTool *s_instance;

    OSGWidget *m_osg_widget;
    eType m_current_type;
};

} // namespace matisse

#endif // MATISSE_OSG_WIDGET_TOOL_H_
