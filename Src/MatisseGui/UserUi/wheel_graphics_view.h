#ifndef MATISSE_WHEEL_GRAPHICS_VIEW_H_
#define MATISSE_WHEEL_GRAPHICS_VIEW_H_

#include <QGraphicsView>

namespace matisse {

// QGraphicsView with
//      Zoom with mouse wheel
//      Pan with middle button
//
class WheelGraphicsView : public QGraphicsView
{
    Q_OBJECT

 public:
    explicit WheelGraphicsView(QWidget *_parent = Q_NULLPTR) : QGraphicsView(_parent), m_zoom(1.0)
    {
        setDragMode(QGraphicsView::NoDrag);
        this->setMouseTracking(true);
    }
    void setZoomFactor(qreal _zoom, const QPoint * _opt_mouse_pos = NULL);
    qreal zoomfactor() const { return m_zoom; }

 signals:
    void si_updateCoords(QPointF _p);
    void si_zoomChanged(qreal _zoom);
    void si_panChanged();

 protected:
    virtual void wheelEvent(QWheelEvent* _event) override;
    virtual void mousePressEvent(QMouseEvent *_event) override;
    virtual void mouseMoveEvent(QMouseEvent *_event) override;
    virtual void mouseReleaseEvent(QMouseEvent *_event) override;

private:
    QPointF m_reference;
    QPointF m_center_view;

    qreal m_zoom;
};

} // namespace matisse

#endif // MATISSE_WHEEL_GRAPHICS_VIEW_H_
