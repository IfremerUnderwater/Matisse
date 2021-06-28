#ifndef MATISSE_SOURCE_WIDGET_H_
#define MATISSE_SOURCE_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class SourceWidget : public ElementWidget
{
//    Q_OBJECT

public:
    SourceWidget(QGraphicsItem *_parent = 0);
    virtual void drawSymbol(QPainter *_painter, bool _for_icon = false);

    virtual int type() const { return SOURCE_TYPE;}

protected:


private:
    QPointF m_ray_init;
    QPointF m_ray_end;

signals:

public slots:

};

} // namespace matisse

#endif // MATISSE_SOURCE_WIDGET_H_
