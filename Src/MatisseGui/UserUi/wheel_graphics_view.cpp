#include "wheel_graphics_view.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <math.h>

namespace matisse {

void WheelGraphicsView::setZoomFactor(qreal _zoom, const QPoint * _opt_mouse_pos)
 {
    // limits for zooming
    if ((_zoom != m_zoom)&&(_zoom >= 0.01f)&&(_zoom <= 100.0f))
    {
       QPointF old_pos;
       if (_opt_mouse_pos) old_pos = mapToScene(*_opt_mouse_pos);

       // Remember what point we were centered on before...
       m_zoom = _zoom;
       QMatrix m;
       m.scale(m_zoom, m_zoom);
       setMatrix(m);

       if (_opt_mouse_pos)
       {
          const QPointF new_pos = mapFromScene(old_pos);
          const QPointF move   = new_pos-*_opt_mouse_pos;
          horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
          verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());
       }
       emit si_zoomChanged(m_zoom);
    }
 }

 void WheelGraphicsView::wheelEvent(QWheelEvent* _event)
 {
    QPoint pos = _event->pos();
    setZoomFactor(m_zoom*pow(1.2, _event->delta() / 240.0), &pos);
    _event->accept();
 }

 void WheelGraphicsView::mousePressEvent(QMouseEvent *_event)
 {
     if (_event->button() == Qt::MiddleButton)
     {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        m_reference = mapToScene(_event->pos());
        m_center_view = mapToScene(this->viewport()->rect()).boundingRect().center();
        _event->accept();
        return;
     }
     QGraphicsView::mousePressEvent(_event);
 }

 void WheelGraphicsView::mouseMoveEvent(QMouseEvent *_event)
 {
     // coords
     QPointF pt = mapToScene(_event->pos());
     emit si_updateCoords(pt);

     // pan the chart with a middle mouse drag
     if (_event->buttons() & Qt::MiddleButton)
     {
         QPointF move = m_reference - pt;
         centerOn(m_center_view + move);
         m_center_view = mapToScene(this->viewport()->rect()).boundingRect().center();
         _event->accept();

         emit si_panChanged();

         return;
     }

     QGraphicsView::mouseMoveEvent(_event);
 }

 void WheelGraphicsView::mouseReleaseEvent(QMouseEvent *_event) {
     if (_event->button() == Qt::MidButton) {
         QApplication::restoreOverrideCursor();
         return;
     }

     QGraphicsView::mouseReleaseEvent(_event);
 }

} // namespace matisse
