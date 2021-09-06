#ifndef MATISSE_PIPE_WIDGET_H_
#define MATISSE_PIPE_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class PipeWidget : public ElementWidget
{

public:
    PipeWidget(QGraphicsItem *_parent = 0);

    virtual void drawSymbol(QPainter *_painter, bool _for_icon = false);
    void setGeometry(int _x, int _y, int _delta_y_t = 0, int _delta_x_t = 0, int _delta_y = 0, int _delta_x_b = 0, int _delta_y_b = 0, QColor _color = QColor());
    void setColor(QColor _color);
    void fromSourcePos(QPointF _src_pos);
    void toDestinationPos(QPointF _dst_pos);
    virtual int type() const { return PIPE_TYPE;}

    void setStart(bool _temp, ElementWidget * _src, int _src_line);
    void setEnd(ElementWidget *_dest, int _dest_line);
    ElementWidget * getStartElement();
    ElementWidget *getTempStartElement();
    ElementWidget * getEndElement();
    int getStartElementLine();
    int getEndElementLine();
    void refreshConnections();

    virtual void clone(PipeWidget * _other);

    static void setCurrentColor(QRgb _color);
    static QRgb getNextColor();
    bool hasStart();
    void clear();

private:
    static int m_current_color_index;
    static QList<QRgb> m_colors_list;
    static void fillColorsList();

    int m_x;
    int m_y;
    int m_delta_y_t;
    int m_delta_x_t;
    int m_delta_y;
    int m_delta_x_b;
    int m_delta_y_b;

    ElementWidget * m_start_elt;
    ElementWidget * m_temp_start_elt;
    ElementWidget * m_end_elt;
    int m_start_line;
    int m_end_line;

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_PIPE_WIDGET_H_
