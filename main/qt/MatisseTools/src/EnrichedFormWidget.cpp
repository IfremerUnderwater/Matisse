#include "EnrichedFormWidget.h"

using namespace MatisseTools;

QSharedPointer<QFontMetrics> EnrichedFormWidget::_metrics;

EnrichedFormWidget::EnrichedFormWidget(QWidget *parent) :
    QWidget(parent)
{
    if(_metrics == NULL) {
        _metrics = QSharedPointer<QFontMetrics>(new QFontMetrics(QLabel().font()));
    }
    _label = new QLabel(this);
    _gridLayout = new QGridLayout(this);
    _gridLayout->setContentsMargins(0, 0, 0, 0);
}

void EnrichedFormWidget::setWidget(QString label, QWidget *widget)
{
    if (!label.isEmpty()) {
        label.append(":");
    }
    _label->setText(label);
    _widget = widget;
    _gridLayout->addWidget(_label, 0, 0);
    _gridLayout->addWidget(widget, 0, 1);
    if (widget -> minimumHeight() > 40) {
        _gridLayout->setAlignment(_label, Qt::AlignTop | Qt::AlignLeft);
    }
    _gridLayout->setAlignment(widget, Qt::AlignLeft);
    _gridLayout->setRowStretch(0, 1);
    _gridLayout->setRowStretch(0, 1);
}

quint32 EnrichedFormWidget::getLabelWidth()
{
    return _metrics->width(_label->text());
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

void EnrichedFormWidget::slot_valueChanged()
{
    swapColor(currentValueChanged());
    emit signal_valueChanged(currentValueChanged());
}

void EnrichedFormWidget::setLabelColumnWidth(quint32 width)
{
    _gridLayout->setColumnMinimumWidth(0, width);
}

quint32 EnrichedFormWidget::getLabelColumnWidth()
{
    return _gridLayout->columnMinimumWidth(0);
}

void EnrichedFormWidget::setWidth(quint32 labelWidth, quint32 widgetWidth) {
    _gridLayout->setColumnMinimumWidth(0, labelWidth);
    _gridLayout->setColumnMinimumWidth(1, widgetWidth);
    setMinimumWidth(labelWidth + widgetWidth);
    setMaximumWidth(labelWidth + widgetWidth);
}
