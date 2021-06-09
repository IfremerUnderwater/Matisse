#include "pipe_widget.h"

namespace matisse {

int PipeWidget::m_current_color_index = 0;
QList<QRgb> PipeWidget::m_colors_list = QList<QRgb>();

PipeWidget::PipeWidget(QGraphicsItem *_parent):ElementWidget(_parent),
    m_start_elt(0),
    m_temp_start_elt(0),
    m_end_elt(0),
    m_start_line(-1),
    m_end_line(-1)
{
    fillColorsList();
    qDebug() << "CONSTRUCTION PIPEWIDGET";
    setGeometry(0, 0);
    setZValue(1.0);
}

bool PipeWidget::hasStart() {
    return (m_temp_start_elt != 0);
}

void PipeWidget::clear()
{
    m_start_elt = 0;
    m_temp_start_elt = 0;
    m_end_elt = 0;
    m_start_line = -1;
    m_end_line = -1;
    setGeometry(0, 0);
    setVisible(false);
}

void PipeWidget::fillColorsList() {
    if (!m_colors_list.isEmpty()) {
        return;
    }
    for (int index =4; index < 19; index++) {
        m_colors_list.append(QColor(static_cast<Qt::GlobalColor>(index)).rgba());
    }
    m_current_color_index = 0;
}

QRgb PipeWidget::getNextColor()
{
    m_current_color_index++;
    if (m_current_color_index == m_colors_list.size()) {
        m_current_color_index = 0;
    }
    return m_colors_list[m_current_color_index];
}

void PipeWidget::setCurrentColor(QRgb _color)
{
    int index = m_colors_list.indexOf(_color);
    m_current_color_index = qMax(0, index);
}

void PipeWidget::drawSymbol(QPainter *_painter, bool _for_icon)
{
    Q_UNUSED(_for_icon)

    // connecteur sup
    //qDebug() << "DRAW PIPE" << this;
    _painter->setPen(m_color);
    _painter->setBrush(QBrush(m_color));
    QRect src(m_x - 6, m_y - 6, 12, 12);
    QRect dst(m_x + m_delta_x_t + m_delta_x_b -6, m_y + m_delta_y_t + m_delta_y + m_delta_y_b -6, 12, 12);

    _painter->drawArc(src,-180*16, 180*16);

    // Connect modules with bezier curve
    QPainterPath my_path;
    int vertical_dist = m_delta_y_t + m_delta_y + m_delta_y_b -6;
    my_path.moveTo(m_x, m_y +6);
    my_path.cubicTo(m_x, m_y + vertical_dist/2.0 ,
                   m_x + m_delta_x_t + m_delta_x_b, m_y + m_delta_y_t + m_delta_y + m_delta_y_b -vertical_dist/2.0,
                   m_x + m_delta_x_t + m_delta_x_b , m_y + m_delta_y_t + m_delta_y + m_delta_y_b -6);

    _painter->setBrush(QColor(Qt::transparent));
    _painter->drawPath(my_path);

    // connecteur inf
    _painter->drawEllipse(dst);

}

void PipeWidget::setGeometry(int _x, int _y, int _delta_y_t, int _delta_x_t, int _delta_y, int _delta_x_b, int _delta_y_b, QColor _color)
{
    m_x = _x;
    m_y = _y;
    m_delta_y_t = _delta_y_t;
    m_delta_x_t = _delta_x_t;
    m_delta_y = _delta_y;
    m_delta_x_b = _delta_x_b;
    m_delta_y_b = _delta_y_b;

    setColor(_color);

}

void PipeWidget::setColor(QColor _color)
{
    if (!_color.isValid()) {
        //_color = QColor(Qt::black);
        m_color = QColor::fromRgb(245, 247, 250); // gris clair de la charte graphique (Matisse.css)
    } else {
        m_color = _color;
    }
    //update();
}

void  PipeWidget::setStart(bool _temp, ElementWidget * _src, int _src_line) {
    //qDebug() << "Set start..." << temp << int(src) << srcLine;
    if (_temp) {
        m_temp_start_elt = _src;
    } else {
        m_start_elt = _src;
    }
    m_start_line = _src_line;

    if (_src && (_src_line > -1)) {
        qDebug() << "src dump";
        _src->dumpObjectInfo();
        qDebug() << "Line" << _src_line;

        QPointF src_pos = mapFromScene(_src->getOutputLinePos(_src_line));
        setGeometry(src_pos.x(), src_pos.y());
        if (!_temp) {
            _src->setOutputWidget(_src_line, this);
        }

       // prepareGeometryChange();
        m_bounding_rect = QRectF(m_x-6, m_y-6, 12, 12);
    } else {
        qDebug() << "Sortie setStart 0";
    }
}

void PipeWidget::setEnd(ElementWidget * _dest, int _dest_line)
{
    m_end_elt = _dest;
    m_end_line = _dest_line;
    if (_dest && (_dest_line > -1)) {
        setStart(false, m_temp_start_elt, m_start_line);
        toDestinationPos(_dest->getInputLinePos(_dest_line));
        _dest->setInputWidget(_dest_line, this);
    }
}

ElementWidget *PipeWidget::getStartElement()
{
    return m_start_elt;
}

ElementWidget *PipeWidget::getTempStartElement()
{
    return m_temp_start_elt;
}

ElementWidget *PipeWidget::getEndElement()
{
    return m_end_elt;
}

int PipeWidget::getStartElementLine()
{
    return m_start_line;
}

int PipeWidget::getEndElementLine()
{
    return m_end_line;
}

void PipeWidget::refreshConnections()
{
    setStart(true, m_start_elt, m_start_line);
    setEnd(m_end_elt, m_end_line);
}

void PipeWidget::fromSourcePos(QPointF _src_pos)
{
    _src_pos = mapFromScene(_src_pos);
    m_x = _src_pos.x();
    m_y = _src_pos.y();
    qDebug() << "from.............................................................";
}

void PipeWidget::toDestinationPos(QPointF _dst_pos)
{
    _dst_pos = mapFromScene(_dst_pos);
    int dst_x = _dst_pos.x();
    int dst_y = _dst_pos.y();

    bool big_step = false;
    if (m_start_elt && m_end_elt) {
        int start_order = m_start_elt->getOrder();
        int end_order = m_end_elt->getOrder();
        int start_x_pos = m_start_elt->getOutputLinePos(m_start_line).x();
        int start_y_pos = m_start_elt->getOutputLinePos(m_start_line).y();
        int end_x_pos = m_end_elt->getInputLinePos(m_end_line).x();
        int end_y_pos = m_end_elt->getInputLinePos(m_end_line).y();

        if ((end_order - start_order) != 1) {
            big_step = true;
            m_delta_y_t = m_start_line * 4 + 10;
            m_delta_y_b = m_end_line * 4 + 10;
            m_delta_x_t = start_x_pos - 200 + start_order * 20 + m_start_line * 5;
            m_delta_x_b = end_x_pos - start_x_pos - m_delta_x_t;
            m_delta_y = end_y_pos - start_y_pos - m_delta_y_t - m_delta_y_b;

            // calcul bounding rect...
        }
    }
    if (!big_step) {
        m_delta_y_t = qBound(-10, dst_y -m_y, 10);
        m_delta_y_b = qBound(-10, dst_y -m_y - m_delta_y_b, 10);
        m_delta_y = _dst_pos.y() - (m_y + m_delta_y_t + m_delta_y_b);
        m_delta_x_t = (dst_x - m_x)/2;
        m_delta_x_b = dst_x - m_delta_x_t -m_x;
    }

    //setVisible(true);
    if (m_start_elt) {
        double left_x = qMin(m_x + m_delta_x_t, m_x + m_delta_x_t + m_delta_x_b);
        double right_x = qMax(m_x + m_delta_x_t, m_x + m_delta_x_t + m_delta_x_b);
        QPointF loc_src_pos = mapFromScene(m_start_elt->getOutputLinePos(m_start_line));
        QPointF loc_dst_pos = mapFromScene(_dst_pos);
        prepareGeometryChange();
        m_bounding_rect = QRectF(loc_src_pos.x()-6, loc_src_pos.y()-6, 12, 12);
        m_bounding_rect = m_bounding_rect.united(QRectF(loc_dst_pos.x(), loc_dst_pos.y(), 6, 6));
        m_bounding_rect = m_bounding_rect.united(QRectF(left_x, loc_src_pos.y(), right_x - left_x, 1));
    }
}

void PipeWidget::clone(PipeWidget *_other)
{
    ElementWidget::clone(_other);

    m_color = _other->m_color;
    m_x = _other->m_x;
    m_y = _other->m_y;
    m_delta_y_t = _other->m_delta_y_t;
    m_delta_x_t = _other->m_delta_x_t;
    m_delta_y = _other->m_delta_y;
    m_delta_x_b = _other->m_delta_x_b;
    m_delta_y_b = _other->m_delta_y_b;
    m_start_elt = _other->m_start_elt;
    m_temp_start_elt = _other->m_temp_start_elt;
    m_end_elt = _other->m_end_elt;
    m_start_line = _other->m_start_line;
    m_end_line = _other->m_end_line;
}

} // namespace matisse
