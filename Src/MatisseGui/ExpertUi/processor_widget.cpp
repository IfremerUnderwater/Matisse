#include "processor_widget.h"

namespace matisse {

void ProcessorWidget::drawSymbol(QPainter * _painter, bool _for_icon) {

    QRect temp_rect = m_body_rect;
    if (_for_icon) {
        m_body_rect.translate(m_body_rect.width()/2, m_bounding_rect.height()/2);
    }

    _painter->drawRoundedRect(m_body_rect, 5, 5);
    QVector <QLine> lines_in;
    QVector <QLine> lines_out;
    int proc_width = m_body_rect.width();
    int x_orig = m_body_rect.left();
    int y_orig = m_body_rect.top();
    int delta_in = proc_width / (m_inputs_number + 1);
    int delta_out = proc_width / (m_outputs_number + 1);

    for (int index = 1; index <= m_inputs_number; index++) {
        lines_in << QLine(x_orig + delta_in * index, y_orig, x_orig + delta_in * index, y_orig -30);
        _painter->drawArc(QRect(x_orig + delta_in * index - 6, y_orig -42, 12, 12),-180*16, 180*16);
    }

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

    _painter->drawLines(lines_in);
    _painter->drawLines(lines_out);
    _painter->drawText(m_body_rect, Qt::AlignCenter, m_name);

    m_body_rect = temp_rect;
}

} // namespace matisse
