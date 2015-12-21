#include "ParametersWidget.h"

using namespace MatisseServer;

void ParametersWidget::drawSymbol(QPainter * painter, bool forIcon) {

    QRect tempRect = _bodyRect;
    if (forIcon) {
        _bodyRect.translate(_bodyRect.width()/2, _bodyRect.height()/2);
        QFont font = painter->font();
        font.setPointSize(font.pointSize()+5);
        painter->setFont(font);
    }

        int srcWidth = _bodyRect.width();
        //int xOrig = _processorRect.x() /*+ 5*/;
        int xLeft = _bodyRect.left();
        int xRight = _bodyRect.right();
        int yTop = _bodyRect.top()+10;
        int yBottom = _bodyRect.bottom()-10;

        painter->drawLine(xLeft, yTop, xLeft, yBottom);
        painter->drawLine(xRight, yTop, xRight, yBottom);
//        painter->drawEllipse(QRect(xLeft, yTop-10, srcWidth, 20));
        painter->drawArc(QRect(xLeft, yTop-10, srcWidth-1, 20), 180*16, -180*16);
        painter->drawText(_bodyRect, Qt::AlignCenter, _name);
//        painter->drawText(QRect(xLeft, yTop, xRight, yBottom), Qt::AlignCenter, _name);
        painter->drawArc(QRect(xLeft, yBottom-10, srcWidth-1, 20),-180*16, 180*16);

        _bodyRect = tempRect;
}
