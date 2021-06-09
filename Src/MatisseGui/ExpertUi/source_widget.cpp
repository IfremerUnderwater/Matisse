#include "source_widget.h"

namespace matisse {

SourceWidget::SourceWidget(QGraphicsItem *_parent):ElementWidget(_parent)
{
    setOrder(0);
}

void SourceWidget::drawSymbol(QPainter * _painter, bool _for_icon) {

    QRect temp_rect = m_body_rect;
    if (_for_icon) {
        m_body_rect.translate(m_body_rect.width()/2, m_body_rect.height()/2);
    }

        _painter->drawEllipse(m_body_rect);
        QVector <QLine> lines_out;
        int src_width = m_body_rect.width();
        int x_orig = m_body_rect.left();
        int y_orig = m_body_rect.top();
        int delta_out = src_width / (m_outputs_number + 1);

        for (int index = 1; index <= m_outputs_number; index++) {
            ElementWidget * output_widget = m_outputs_widgets.at(index-1);
            if (output_widget) {
                _painter->setBrush(QBrush(output_widget->getColor()));
            } else {
                _painter->setBrush(QBrush());
            }
            lines_out << QLine(x_orig + delta_out * index, y_orig + m_body_rect.height(), x_orig + delta_out * index, y_orig + m_body_rect.height() +30);
            _painter->drawEllipse(QRect(x_orig + delta_out * index - 6, y_orig + m_body_rect.height() +30, 12, 12));
        }

        _painter->drawLines(lines_out);
        _painter->drawText(m_body_rect, Qt::AlignCenter, m_name);


    m_body_rect = temp_rect;
}


} // namespace matisse
