#include "element_widget.h"
#include "assembly_graphics_scene.h"

namespace matisse {

ElementWidget::ElementWidget(QGraphicsItem *_parent) :
    QObject(0),
    QGraphicsItem(_parent)
{
    m_body_rect = QRect(-70, -30, 140, 60);
    m_brush.setColor(Qt::white);
    m_pen.setColor(QColor::fromRgb(245, 247, 250)); // gris clair de la charte graphique (Matisse.css)
    setInputsNumber();
    setOutputsNumber();
    setAcceptHoverEvents(true);
    setFlags(ItemIsFocusable | ItemIsSelectable /*| ItemIsMovable*/);
    m_button_pressed = false;
    m_show_frame = false;
    m_start_move = QPointF(0,0);
    m_color = QColor();
    setOrder();
}

void ElementWidget::setName(QString _name) {
    m_name = _name;
}

void ElementWidget::clone(ElementWidget * _other)
{
    if (!_other) {
        return;
    }

    m_inputs_number = _other -> m_inputs_number;
    m_outputs_number = _other -> m_outputs_number;
    m_body_rect = _other -> m_body_rect;
    m_bounding_rect = _other -> m_bounding_rect;
    m_brush = _other -> m_brush;
    m_pen = _other -> m_pen;
    m_name = _other -> m_name;
    m_button_pressed = _other -> m_button_pressed;
    m_inputs_zones = _other -> m_inputs_zones;
    m_outputs_zones = _other -> m_outputs_zones;
    m_inputs_widgets = _other->m_inputs_widgets;
    m_outputs_widgets = _other->m_outputs_widgets;
    m_elt_order = _other->m_elt_order;
    m_color = _other->m_color;
}

QString ElementWidget::getName() {
    return m_name;
}

int ElementWidget::getInputLine(QPointF _pos)
{
    if (_pos.isNull()) {
        return -1;
    }

    int ret = -1;

    for (int index = 0; index < m_inputs_zones.length(); index++) {
        QPointF pos_temp = mapFromScene(_pos);
        if (!m_inputs_zones[index].isNull()) {
            pos_temp -= m_inputs_zones[index];
            if (pos_temp.manhattanLength() < 9.0) {
                ret = index;
                break;
            }
        }
    }

    return ret;
}

int ElementWidget::getOutputLine(QPointF _pos)
{
    if (_pos.isNull()) {
        return -1;
    }

    int ret = -1;

    for (int index = 0; index < m_outputs_zones.length(); index++) {
        QPointF pos_temp = mapFromScene(_pos);
        if (!m_outputs_zones[index].isNull()) {
            pos_temp -= m_outputs_zones[index];
            if (pos_temp.manhattanLength() < 9.0) {
                ret = index;
                break;
            }
        }
    }

    return ret;
}

void ElementWidget::setColor(QColor _color)
{
    m_color = _color;
}

QColor ElementWidget::getColor()
{
    return m_color;
}

QPointF ElementWidget::getInputLinePos(quint8 _input_line)
{
    if (_input_line >= m_inputs_number) {
        return QPointF();
    }
    return mapToScene(m_inputs_zones[_input_line]);
}

QPointF ElementWidget::getOutputLinePos(quint8 _output_line)
{
    if (_output_line >= m_outputs_number) {
        return QPointF();
    }
    return mapToScene(m_outputs_zones[_output_line]);
}

bool ElementWidget::setInputWidget(quint8 _no_line, ElementWidget *_element)
{
    if (_no_line >= m_inputs_number) {
        return false;
    }
    ElementWidget * elt = m_inputs_widgets.at(_no_line).data();
    if (elt) {
        delete elt;
    }
    m_inputs_widgets[_no_line] = _element;

    return true;
}

bool ElementWidget::setOutputWidget(quint8 _no_line, ElementWidget * _element)
{
    if (_no_line >= m_outputs_number) {
        return false;
    }
    ElementWidget * elt = m_outputs_widgets.at(_no_line).data();
    if (elt) {
        delete elt;
    }
    m_outputs_widgets[_no_line] = _element;

    return true;
}

ElementWidget * ElementWidget::getInputWidget(quint8 _no_line)
{
    if (_no_line >= m_inputs_number) {
        return 0;
    }

    return  m_inputs_widgets.at(_no_line);
}

ElementWidget * ElementWidget::getOutputWidget(quint8 _no_line)
{
    if (_no_line >= m_outputs_number) {
        return 0;
    }

    return  m_outputs_widgets.at(_no_line);
}

void ElementWidget::computeBoundingRect()
{
    m_bounding_rect = m_body_rect;
    if (m_inputs_number > 0) {
        m_bounding_rect.setTop(m_body_rect.top()-38);
    }
    if (m_outputs_number > 0) {
        m_bounding_rect.setBottom(m_body_rect.bottom() + 44);
    }
}

QIcon ElementWidget::getIcon()
{
    QRectF elt_rect(m_bounding_rect);
    elt_rect.adjust(0, 0, 1, 0);
    QPixmap image(elt_rect.width(), elt_rect.height());
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setPen(m_pen);
    drawSymbol(&painter, true);

    return QIcon(image);
}

void ElementWidget::setInputsNumber(int _number)
{
    m_inputs_number = _number;

    m_inputs_widgets.resize(_number);
//    _inputsWidgets.clear();

    int proc_width = m_body_rect.width();
    int x_orig = m_body_rect.left();
    int y_orig = m_body_rect.top();
    int delta_in = proc_width / (m_inputs_number + 1);

    for (int index = 1; index <= m_inputs_number; index++) {
        m_inputs_zones << QPointF(x_orig + delta_in * index, y_orig - 36);
    }

    computeBoundingRect();
}

void ElementWidget::setOutputsNumber(int _number)
{
    m_outputs_number = _number;

    m_outputs_widgets.resize(_number);

    int src_width = m_body_rect.width();
    int x_orig = m_body_rect.left();
    int y_orig = m_body_rect.top();
    int delta_out = src_width / (m_outputs_number + 1);

    for (int index = 1; index <= m_outputs_number; index++) {
        m_outputs_zones << QPointF(x_orig + delta_out * index, y_orig + m_body_rect.height() + 36);
    }

    computeBoundingRect();
}

void ElementWidget::paint(QPainter *_painter, const QStyleOptionGraphicsItem *_option, QWidget *_widget)
{
    Q_UNUSED(_option)
    Q_UNUSED(_widget)

    _painter->save();
    _painter->setPen(m_pen);
    _painter->setBrush(m_brush);

    QFont font = _painter->font();
    font.setPointSize(font.pointSize()-1);
    _painter->setFont(font);

    drawSymbol(_painter);

    // pour debug m_show_frame = true
    //m_show_frame = true;
    if (m_show_frame) {
        _painter->setBrush(Qt::NoBrush);
        _painter->setPen(Qt::red);
        _painter->drawRect(m_bounding_rect);
    }

    _painter->restore();

}

QRectF ElementWidget::boundingRect() const
{
    return m_bounding_rect;
}

void ElementWidget::setOrder(qint8 _order)
{
    m_elt_order = _order;
}

qint8 ElementWidget::getOrder()
{
    return m_elt_order;
}

void ElementWidget::dragEnterEvent(QGraphicsSceneDragDropEvent *_event)
{
    Q_UNUSED(_event)
}


} // namespace matisse
