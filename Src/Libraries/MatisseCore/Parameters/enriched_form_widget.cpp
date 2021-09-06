#include "enriched_form_widget.h"
#include "graphical_charter.h"

namespace matisse {

QSharedPointer<QFontMetrics> EnrichedFormWidget::m_metrics;

EnrichedFormWidget::EnrichedFormWidget(QWidget *_parent) :
    QWidget(_parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    if(m_metrics == NULL) {
        m_metrics = QSharedPointer<QFontMetrics>(new QFontMetrics(QFont(MATISSE_FONT_TYPE, MATISSE_FONT_DEFAULT_SIZE_PT)));
    }
    m_label = new QLabel(this);
    m_grid_layout = new QGridLayout(this);
    m_grid_layout->setContentsMargins(0, 0, 0, 0);
    m_grid_layout->setHorizontalSpacing(graph_chart.dpiScaled(PARAM_WIDGET_FIELD_HSPACING));
}

void EnrichedFormWidget::setValue(QString _new_value)
{
    applyValue(_new_value);
    m_initial_value = currentValue();
}

void EnrichedFormWidget::setWidget(QString _label, QWidget *_widget, bool _wrap_widget)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();

    if (!_label.isEmpty()) {
        _label.append(":");
    }
    m_label->setText(_label);
    m_label->setWordWrap(true);

    m_widget = _widget;
    if (_wrap_widget) {
        m_label->setFixedWidth(graph_chart.dpiScaled(PARAM_LABEL_WIDTH_WRAP));
        /* widget is displayed on 2nd row */
        m_grid_layout->addWidget(m_label, 0, 0);
        m_grid_layout->addWidget(_widget, 1, 0);
        //_gridLayout->setAlignment(widget, Qt::AlignRight);
        m_grid_layout->setAlignment(_widget, Qt::AlignLeft);
    } else {
        m_label->setFixedWidth(graph_chart.dpiScaled(PARAM_LABEL_WIDTH_NOWRAP));
        m_grid_layout->addWidget(m_label, 0, 0);
        m_grid_layout->addWidget(_widget, 0, 1);
        m_grid_layout->setAlignment(_widget, Qt::AlignLeft);
    }
    if (_widget -> minimumHeight() > graph_chart.dpiScaled(PARAM_WIDGET_ALIGN_HEIGHT_THRE)) {
        m_grid_layout->setAlignment(m_label, Qt::AlignTop | Qt::AlignLeft);
    }
}

bool EnrichedFormWidget::currentValueChanged()
{
    QString current_val = currentValue();
    return current_val != m_initial_value;
}

void EnrichedFormWidget::swapColor(bool _do_swap)
{
    QPalette label_palette = m_label->palette();
    QPalette wid_palette = m_widget->palette();

    if (_do_swap) {
        label_palette.setColor(QPalette::WindowText, Qt::red);
        m_label->setPalette(label_palette);
        wid_palette.setColor(QPalette::WindowText, Qt::red);
        m_widget->setPalette(wid_palette);
    } else {
        label_palette.setColor(QPalette::WindowText, Qt::black);
        m_label->setPalette(label_palette);
        wid_palette.setColor(QPalette::WindowText, Qt::black);
        m_widget->setPalette(wid_palette);
    }
}

quint32 EnrichedFormWidget::getTextFieldWidth(QString _text)
{
    return m_metrics->width(_text);
}

void EnrichedFormWidget::sl_valueChanged()
{
    bool has_value_changed = currentValueChanged();
    swapColor(has_value_changed);
    emit si_valueChanged(has_value_changed);
}

} // namespace matisse
