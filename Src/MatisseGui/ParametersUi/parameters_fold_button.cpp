#include "ParametersFoldButton.h"

using namespace MatisseServer;

const quint8 ParametersFoldButton::ARROW_LINE_WIDTH = 8;
const quint8 ParametersFoldButton::ARROW_LINE_HEIGHT = 10;
const quint8 ParametersFoldButton::ARROW_TIP_WIDTH = 12;
const quint8 ParametersFoldButton::ARROW_WING_HEIGHT = 7;

ParametersFoldButton::ParametersFoldButton(QWidget *parent) :
    FoldUnfoldButton(parent)
{

}

void ParametersFoldButton::paintEvent(QPaintEvent *event)

{
    // draw widget as regular QButton + stylesheet
    QPushButton::paintEvent(event);

    // customize paint to draw fold/unfold arrow
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    //QColor matisseBlack(MATISSE_BLACK);
    QColor paintColor(_currentPaintColorRef);

    painter.setPen(paintColor);
    painter.setBrush(paintColor);

    if (getIsUnfolded()) {
        drawFoldArrow(painter, paintColor);
    } else {
        drawUnfoldArrow(painter, paintColor);
    }
}

void ParametersFoldButton::drawFoldArrow(QPainter &painter, QColor paintColor)
{
    quint8 arrowTipHeight = ARROW_LINE_HEIGHT + 2 * ARROW_WING_HEIGHT;

    quint16 arrowLineLeft = 0;
    quint16 arrowLineTop = height() - ARROW_LINE_HEIGHT - ARROW_WING_HEIGHT;

    quint16 arrowTipLeft = arrowLineLeft + ARROW_LINE_WIDTH;
    quint16 arrowTipTop = arrowLineTop - ARROW_WING_HEIGHT;

    QRect arrowLine(arrowLineLeft, arrowLineTop, ARROW_LINE_WIDTH, ARROW_LINE_HEIGHT);
    painter.drawRect(arrowLine);

    QRectF arrowTipRect = QRectF(arrowTipLeft, arrowTipTop, ARROW_TIP_WIDTH, arrowTipHeight);

    QPainterPath arrowTipPath;
    arrowTipPath.moveTo(arrowTipRect.topLeft());
    arrowTipPath.lineTo(arrowTipRect.right(), arrowTipRect.top() + (arrowTipRect.height() / 2));
    arrowTipPath.lineTo(arrowTipRect.bottomLeft());
    arrowTipPath.lineTo(arrowTipRect.topLeft());

    painter.fillPath(arrowTipPath, paintColor);
}

void ParametersFoldButton::drawUnfoldArrow(QPainter &painter, QColor paintColor)
{
    quint8 arrowTipHeight = ARROW_LINE_HEIGHT + 2 * ARROW_WING_HEIGHT;

    quint16 arrowLineLeft = ARROW_TIP_WIDTH;
    quint16 arrowLineTop = height() - ARROW_LINE_HEIGHT - ARROW_WING_HEIGHT;

    quint16 arrowTipLeft = 0;
    quint16 arrowTipTop = arrowLineTop - ARROW_WING_HEIGHT;

    QRect arrowLine(arrowLineLeft, arrowLineTop, ARROW_LINE_WIDTH, ARROW_LINE_HEIGHT);
    painter.drawRect(arrowLine);

    QRectF arrowTipRect = QRectF(arrowTipLeft, arrowTipTop, ARROW_TIP_WIDTH, arrowTipHeight);

    QPainterPath arrowTipPath;
    arrowTipPath.moveTo(arrowTipRect.topRight());
    arrowTipPath.lineTo(arrowTipRect.left(), arrowTipRect.top() + (arrowTipRect.height() / 2));
    arrowTipPath.lineTo(arrowTipRect.bottomRight());
    arrowTipPath.lineTo(arrowTipRect.topRight());

    painter.fillPath(arrowTipPath, paintColor);
}

void ParametersFoldButton::showNewState()
{
    repaint();
}

void ParametersFoldButton::slot_updateColorPalette(QMap<QString, QString> newColorPalette)
{
    _currentPaintColorRef = newColorPalette.value("color.black");
    qDebug() << "Parameters fold button, new black color : " << _currentPaintColorRef;
    repaint();
}

