#include "EnrichedFormWidget.h"
#include "GraphicalCharter.h"

using namespace MatisseTools;

QSharedPointer<QFontMetrics> EnrichedFormWidget::_metrics;

EnrichedFormWidget::EnrichedFormWidget(QWidget *parent) :
    QWidget(parent)
{
    MatisseCommon::GraphicalCharter &graph_chart = MatisseCommon::GraphicalCharter::instance();

    if(_metrics == NULL) {
        _metrics = QSharedPointer<QFontMetrics>(new QFontMetrics(QFont(MATISSE_FONT_TYPE, MATISSE_FONT_DEFAULT_SIZE_PT)));
    }
    _label = new QLabel(this);
    _gridLayout = new QGridLayout(this);
    _gridLayout->setContentsMargins(0, 0, 0, 0);
    _gridLayout->setHorizontalSpacing(graph_chart.dpiScaled(PARAM_WIDGET_FIELD_HSPACING));
}

void EnrichedFormWidget::setValue(QString newValue)
{
    applyValue(newValue);
    _initialValue = currentValue();
}

void EnrichedFormWidget::setWidget(QString label, QWidget *widget, bool wrapWidget)
{
    MatisseCommon::GraphicalCharter &graph_chart = MatisseCommon::GraphicalCharter::instance();

    if (!label.isEmpty()) {
        label.append(":");
    }
    _label->setText(label);
    _label->setWordWrap(true);

    _widget = widget;
    if (wrapWidget) {
        _label->setFixedWidth(graph_chart.dpiScaled(PARAM_LABEL_WIDTH_WRAP));
        /* widget is displayed on 2nd row */
        _gridLayout->addWidget(_label, 0, 0);
        _gridLayout->addWidget(widget, 1, 0);
        //_gridLayout->setAlignment(widget, Qt::AlignRight);
        _gridLayout->setAlignment(widget, Qt::AlignLeft);
    } else {
        _label->setFixedWidth(graph_chart.dpiScaled(PARAM_LABEL_WIDTH_NOWRAP));
        _gridLayout->addWidget(_label, 0, 0);
        _gridLayout->addWidget(widget, 0, 1);
        _gridLayout->setAlignment(widget, Qt::AlignLeft);
    }
    if (widget -> minimumHeight() > graph_chart.dpiScaled(PARAM_WIDGET_ALIGN_HEIGHT_THRE)) {
        _gridLayout->setAlignment(_label, Qt::AlignTop | Qt::AlignLeft);
    }
}

bool EnrichedFormWidget::currentValueChanged()
{
    QString currentVal = currentValue();
    return currentVal != _initialValue;
}

void EnrichedFormWidget::swapColor(bool yes)
{
    QPalette labelPalette = _label->palette();
    QPalette widPalette = _widget->palette();

    if (yes) {
        labelPalette.setColor(QPalette::WindowText, Qt::red);
        _label->setPalette(labelPalette);
        widPalette.setColor(QPalette::WindowText, Qt::red);
        _widget->setPalette(widPalette);
    } else {
        labelPalette.setColor(QPalette::WindowText, Qt::black);
        _label->setPalette(labelPalette);
        widPalette.setColor(QPalette::WindowText, Qt::black);
        _widget->setPalette(widPalette);
    }
}

quint32 EnrichedFormWidget::getTextFieldWidth(QString text)
{
    return _metrics->width(text);
}

void EnrichedFormWidget::slot_valueChanged()
{
    bool hasValueChanged = currentValueChanged();
    swapColor(hasValueChanged);
    emit signal_valueChanged(hasValueChanged);
}
