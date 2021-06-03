#include "destination_widget.h"

namespace matisse {

void DestinationWidget::drawSymbol(QPainter *painter, bool forIcon)
{
    QRect tempRect = _bodyRect;
    if (forIcon) {
        _bodyRect.translate(_boundingRect.width()/2, _boundingRect.height()-_bodyRect.height()/2);
    }

    QPoint points[4];
    points[0] = QPoint(_bodyRect.bottomLeft());
    points[1] = QPoint(_bodyRect.right()-15, _bodyRect.bottom());
    points[2] = QPoint(_bodyRect.topRight());
    points[3] = QPoint(_bodyRect.left()+15, _bodyRect.top());

    painter->drawPolygon(points, 4);

    QVector <QLine> linesIn;
    int xOrig = _bodyRect.left();
    int yOrig = _bodyRect.top();

    int deltaIn = _bodyRect.width() / (_inputsNumber + 1);

    for (int index = 1; index <= _inputsNumber; index++) {
        linesIn << QLine(xOrig + deltaIn * index, yOrig, xOrig + deltaIn * index, yOrig -30);
        painter->drawArc(QRect(xOrig + deltaIn * index - 6, yOrig -42, 12, 12),-180*16, 180*16);
    }

    painter->drawLines(linesIn);

    painter->drawText(_bodyRect, Qt::AlignCenter, _name);

    _bodyRect = tempRect;
}

} // namespace matisse
