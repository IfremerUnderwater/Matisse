#include "enriched_decimal_value_widget.h"

using namespace MatisseTools;

EnrichedDecimalValueWidget::EnrichedDecimalValueWidget(QWidget *parent) :
    EnrichedFormWidget(parent),
    _precision(0)
{

}


quint8 EnrichedDecimalValueWidget::precision() const
{
    return _precision;
}

void EnrichedDecimalValueWidget::setPrecision(const quint8 &precision)
{
    _precision = precision;
    applyPrecision();
}

QString EnrichedDecimalValueWidget::withDecimalPrecision(QString matrixCellValue)
{
    double cellValueAsDouble = matrixCellValue.toDouble();
    QString cellValueWithPrecision = QString::number(cellValueAsDouble, 'f', _precision);
    return cellValueWithPrecision;
}
