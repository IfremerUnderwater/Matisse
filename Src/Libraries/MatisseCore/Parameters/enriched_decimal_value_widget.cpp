#include "enriched_decimal_value_widget.h"

namespace matisse {

EnrichedDecimalValueWidget::EnrichedDecimalValueWidget(QWidget *_parent) :
    EnrichedFormWidget(_parent),
    m_precision(0)
{

}


quint8 EnrichedDecimalValueWidget::precision() const
{
    return m_precision;
}

void EnrichedDecimalValueWidget::setPrecision(const quint8 &_precision)
{
    m_precision = _precision;
    applyPrecision();
}

QString EnrichedDecimalValueWidget::withDecimalPrecision(QString _matrix_cell_value)
{
    double cell_value_as_double = _matrix_cell_value.toDouble();
    QString cell_value_with_precision = QString::number(cell_value_as_double, 'f', m_precision);
    return cell_value_with_precision;
}

} // namespace matisse
