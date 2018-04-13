#ifndef WHEELGRAPHICSVIEW_H
#define WHEELGRAPHICSVIEW_H

#include <QGraphicsView>

// QGraphicsView with
//      Zoom with mouse wheel
//      Pan with middle button
//
class WheelGraphicsView : public QGraphicsView
{
    qreal _zoom;

 public:
    WheelGraphicsView(QWidget *parent = Q_NULLPTR) : QGraphicsView(parent), _zoom(1.0)
    {
        setDragMode(QGraphicsView::NoDrag);
        this->setMouseTracking(true);
    }
    void setZoomFactor(qreal zoom, const QPoint * optMousePos = NULL);
    qreal zoomfactor() const { return _zoom; }

 protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPointF _reference;
    QPointF _centerView;
};

#endif // WHEELGRAPHICSVIEW_H
