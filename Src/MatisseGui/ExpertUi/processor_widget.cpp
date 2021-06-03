#include "processor_widget.h"

namespace matisse {

void ProcessorWidget::drawSymbol(QPainter * painter, bool forIcon) {

    QRect tempRect = _bodyRect;
    if (forIcon) {
        _bodyRect.translate(_bodyRect.width()/2, _boundingRect.height()/2);
    }

    painter->drawRoundedRect(_bodyRect, 5, 5);
    QVector <QLine> linesIn;
    QVector <QLine> linesOut;
    int procWidth = _bodyRect.width();
    //int xOrig = _processorRect.x();
    int xOrig = _bodyRect.left();
    int yOrig = _bodyRect.top();
    int deltaIn = procWidth / (_inputsNumber + 1);
    int deltaOut = procWidth / (_outputsNumber + 1);

    for (int index = 1; index <= _inputsNumber; index++) {
        linesIn << QLine(xOrig + deltaIn * index, yOrig, xOrig + deltaIn * index, yOrig -30);
        painter->drawArc(QRect(xOrig + deltaIn * index - 6, yOrig -42, 12, 12),-180*16, 180*16);
    }

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

    painter->drawLines(linesIn);
    painter->drawLines(linesOut);
    painter->drawText(_bodyRect, Qt::AlignCenter, _name);

    _bodyRect = tempRect;
}

} // namespace matisse
