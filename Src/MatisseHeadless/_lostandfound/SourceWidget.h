#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include "element_widget.h"

namespace MatisseServer {
class SourceWidget : public ElementWidget
{
//    Q_OBJECT

public:
    SourceWidget(QGraphicsItem *parent = 0);
    virtual void drawSymbol(QPainter *painter, bool forIcon = false);

    virtual int type() const { return SourceType;}

protected:
//    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
//    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
//    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QPointF _rayInit;
    QPointF _rayEnd;

signals:

public slots:

};
}

#endif // SOURCEWIDGET_H
