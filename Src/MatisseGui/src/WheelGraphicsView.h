#ifndef WHEELGRAPHICSVIEW_H
#define WHEELGRAPHICSVIEW_H

#include <QGraphicsView>

// QGraphicsView with
//      Zoom with mouse wheel
//      Pan with middle button
//
class WheelGraphicsView : public QGraphicsView
{
    Q_OBJECT

 public:
    explicit WheelGraphicsView(QWidget *parent = Q_NULLPTR) : QGraphicsView(parent), _zoom(1.0)
    {
        setDragMode(QGraphicsView::NoDrag);
        this->setMouseTracking(true);
    }
    void setZoomFactor(qreal zoom, const QPoint * optMousePos = NULL);
    qreal zoomfactor() const { return _zoom; }

 signals:
    void signal_updateCoords(QPointF p);
    void signal_zoomChanged(qreal zoom);
    void signal_panChanged();

 protected:
    virtual void wheelEvent(QWheelEvent* event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPointF _reference;
    QPointF _centerView;

    qreal _zoom;
};

#endif // WHEELGRAPHICSVIEW_H
