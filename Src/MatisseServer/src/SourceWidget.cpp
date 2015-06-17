#include "SourceWidget.h"

using namespace MatisseServer;

SourceWidget::SourceWidget(QGraphicsItem *parent):ElementWidget(parent)
{
    setOrder(0);
}

void SourceWidget::drawSymbol(QPainter * painter, bool forIcon) {

    QRect tempRect = _bodyRect;
    if (forIcon) {
        _bodyRect.translate(_bodyRect.width()/2, _bodyRect.height()/2);
    }

        painter->drawEllipse(_bodyRect);
        QVector <QLine> linesOut;
        int srcWidth = _bodyRect.width();
        //int xOrig = _processorRect.x() /*+ 5*/;
        int xOrig = _bodyRect.left();
        int yOrig = _bodyRect.top();
        int deltaOut = srcWidth / (_outputsNumber + 1);

        for (int index = 1; index <= _outputsNumber; index++) {
            ElementWidget * outputWidget = _outputsWidgets.at(index-1);
            if (outputWidget) {
                painter->setBrush(QBrush(outputWidget->getColor()));
            } else {
                painter->setBrush(QBrush());
            }
            linesOut << QLine(xOrig + deltaOut * index, yOrig + _bodyRect.height(), xOrig + deltaOut * index, yOrig + _bodyRect.height() +30);
            painter->drawEllipse(QRect(xOrig + deltaOut * index - 6, yOrig + _bodyRect.height() +30, 12, 12));
        }

        painter->drawLines(linesOut);
        painter->drawText(_bodyRect, Qt::AlignCenter, _name);

//    if (_rayEnd != _rayInit) {
//        painter->drawLine(_rayInit, _rayEnd);
//        qDebug() << "DrawLine..." << _rayInit << _rayEnd;
//    }

    _bodyRect = tempRect;
}

/*
void SourceWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "MousePressEvent" << event->pos() << event->scenePos() << event->screenPos();
    qDebug() << "Modifier" << event->modifiers();
    _rayInit = event->pos();

    event->accept();
}

void SourceWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
//  qDebug() << "MouseMoveElement";
    _rayEnd = event->pos();
    update();
    event->accept();
}

void SourceWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "MouseReleaseEvent";
    _rayEnd = _rayInit;

    event->accept();
}
*/
