#include "wheel_graphics_view.h"
#include <QScrollBar>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QApplication>
#include <math.h>

namespace matisse {

void WheelGraphicsView::setZoomFactor(qreal zoom, const QPoint * optMousePos)
 {
    // limits for zooming
    if ((zoom != _zoom)&&(zoom >= 0.01f)&&(zoom <= 100.0f))
    {
       QPointF oldPos;
       if (optMousePos) oldPos = mapToScene(*optMousePos);

       // Remember what point we were centered on before...
       _zoom = zoom;
       QMatrix m;
       m.scale(_zoom, _zoom);
       setMatrix(m);

       if (optMousePos)
       {
          const QPointF newPos = mapFromScene(oldPos);
          const QPointF move   = newPos-*optMousePos;
          horizontalScrollBar()->setValue(move.x() + horizontalScrollBar()->value());
          verticalScrollBar()->setValue(move.y() + verticalScrollBar()->value());
       }
       emit signal_zoomChanged(_zoom);
    }
 }

 void WheelGraphicsView::wheelEvent(QWheelEvent* event)
 {
    QPoint pos = event->pos();
    setZoomFactor(_zoom*pow(1.2, event->delta() / 240.0), &pos);
    event->accept();
    // not needed : already done in setZoomFactor
    //emit signal_zoomChanged(_zoom);
 }

 void WheelGraphicsView::mousePressEvent(QMouseEvent *event)
 {
     if (event->button() == Qt::MiddleButton)
     {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        _reference = mapToScene(event->pos());
        _centerView = mapToScene(this->viewport()->rect()).boundingRect().center();
        event->accept();
        return;
     }
     QGraphicsView::mousePressEvent(event);
 }

 void WheelGraphicsView::mouseMoveEvent(QMouseEvent *event)
 {
     // coords
     QPointF pt = mapToScene(event->pos());
     emit signal_updateCoords(pt);

     // pan the chart with a middle mouse drag
     if (event->buttons() & Qt::MiddleButton)
     {
         QPointF move = _reference - pt;
         centerOn(_centerView + move);
         _centerView = mapToScene(this->viewport()->rect()).boundingRect().center();
         event->accept();

         emit signal_panChanged();

         return;
     }

     QGraphicsView::mouseMoveEvent(event);
 }

 void WheelGraphicsView::mouseReleaseEvent(QMouseEvent *event) {
     if (event->button() == Qt::MidButton) {
         QApplication::restoreOverrideCursor();
         return;
     }

     QGraphicsView::mouseReleaseEvent(event);
 }

} // namespace matisse
