#include "parameters_fold_button.h"

namespace matisse {

const quint8 ParametersFoldButton::ARROW_LINE_WIDTH = 8;
const quint8 ParametersFoldButton::ARROW_LINE_HEIGHT = 10;
const quint8 ParametersFoldButton::ARROW_TIP_WIDTH = 12;
const quint8 ParametersFoldButton::ARROW_WING_HEIGHT = 7;

ParametersFoldButton::ParametersFoldButton(QWidget *_parent) :
    FoldUnfoldButton(_parent)
{

}

void ParametersFoldButton::paintEvent(QPaintEvent *_event)

{
    // draw widget as regular QButton + stylesheet
    QPushButton::paintEvent(_event);

    // customize paint to draw fold/unfold arrow
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor paint_color(m_current_paint_color_ref);

    painter.setPen(paint_color);
    painter.setBrush(paint_color);

    if (getIsUnfolded()) {
        drawFoldArrow(painter, paint_color);
    } else {
        drawUnfoldArrow(painter, paint_color);
    }
}

void ParametersFoldButton::drawFoldArrow(QPainter &_painter, QColor _paint_color)
{
    quint8 arrow_tip_height = ARROW_LINE_HEIGHT + 2 * ARROW_WING_HEIGHT;

    quint16 arrow_line_left = 0;
    quint16 arrow_line_top = height() - ARROW_LINE_HEIGHT - ARROW_WING_HEIGHT;

    quint16 arrow_tip_left = arrow_line_left + ARROW_LINE_WIDTH;
    quint16 arrow_tip_top = arrow_line_top - ARROW_WING_HEIGHT;

    QRect arrow_line(arrow_line_left, arrow_line_top, ARROW_LINE_WIDTH, ARROW_LINE_HEIGHT);
    _painter.drawRect(arrow_line);

    QRectF arrow_tip_rect = QRectF(arrow_tip_left, arrow_tip_top, ARROW_TIP_WIDTH, arrow_tip_height);

    QPainterPath arrow_tip_path;
    arrow_tip_path.moveTo(arrow_tip_rect.topLeft());
    arrow_tip_path.lineTo(arrow_tip_rect.right(), arrow_tip_rect.top() + (arrow_tip_rect.height() / 2));
    arrow_tip_path.lineTo(arrow_tip_rect.bottomLeft());
    arrow_tip_path.lineTo(arrow_tip_rect.topLeft());

    _painter.fillPath(arrow_tip_path, _paint_color);
}

void ParametersFoldButton::drawUnfoldArrow(QPainter &_painter, QColor _paint_color)
{
    quint8 arrow_tip_height = ARROW_LINE_HEIGHT + 2 * ARROW_WING_HEIGHT;

    quint16 arrow_line_left = ARROW_TIP_WIDTH;
    quint16 arrow_line_top = height() - ARROW_LINE_HEIGHT - ARROW_WING_HEIGHT;

    quint16 arrow_tip_left = 0;
    quint16 arrow_tip_top = arrow_line_top - ARROW_WING_HEIGHT;

    QRect arrow_line(arrow_line_left, arrow_line_top, ARROW_LINE_WIDTH, ARROW_LINE_HEIGHT);
    _painter.drawRect(arrow_line);

    QRectF arrow_tip_rect = QRectF(arrow_tip_left, arrow_tip_top, ARROW_TIP_WIDTH, arrow_tip_height);

    QPainterPath arrow_tip_path;
    arrow_tip_path.moveTo(arrow_tip_rect.topRight());
    arrow_tip_path.lineTo(arrow_tip_rect.left(), arrow_tip_rect.top() + (arrow_tip_rect.height() / 2));
    arrow_tip_path.lineTo(arrow_tip_rect.bottomRight());
    arrow_tip_path.lineTo(arrow_tip_rect.topRight());

    _painter.fillPath(arrow_tip_path, _paint_color);
}

void ParametersFoldButton::showNewState()
{
    repaint();
}

void ParametersFoldButton::sl_updateColorPalette(QMap<QString, QString> _new_color_palette)
{
    m_current_paint_color_ref = _new_color_palette.value("color.black");
    qDebug() << "Parameters fold button, new black color : " << m_current_paint_color_ref;
    repaint();
}

} // namespace matisse
