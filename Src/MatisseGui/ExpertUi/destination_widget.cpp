#include "destination_widget.h"

namespace matisse {

void DestinationWidget::drawSymbol(QPainter *_painter, bool _for_icon)
{
    QRect tempRect = m_body_rect;
    if (_for_icon) {
        m_body_rect.translate(m_bounding_rect.width()/2, m_bounding_rect.height()-m_body_rect.height()/2);
    }

    QPoint points[4];
    points[0] = QPoint(m_body_rect.bottomLeft());
    points[1] = QPoint(m_body_rect.right()-15, m_body_rect.bottom());
    points[2] = QPoint(m_body_rect.topRight());
    points[3] = QPoint(m_body_rect.left()+15, m_body_rect.top());

    _painter->drawPolygon(points, 4);

    QVector <QLine> lines_in;
    int xOrig = m_body_rect.left();
    int yOrig = m_body_rect.top();

    int delta_in = m_body_rect.width() / (m_inputs_number + 1);

    for (int index = 1; index <= m_inputs_number; index++) {
        lines_in << QLine(xOrig + delta_in * index, yOrig, xOrig + delta_in * index, yOrig -30);
        _painter->drawArc(QRect(xOrig + delta_in * index - 6, yOrig -42, 12, 12),-180*16, 180*16);
    }

    _painter->drawLines(lines_in);

    _painter->drawText(m_body_rect, Qt::AlignCenter, m_name);

    m_body_rect = tempRect;
}

} // namespace matisse
