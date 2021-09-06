#ifndef MATISSE_ENRICHED_DECIMAL_VALUE_WIDGET_H_
#define MATISSE_ENRICHED_DECIMAL_VALUE_WIDGET_H_

#include "enriched_form_widget.h"

namespace matisse {

/* Base class for enriched form classes displaying decimal values */
class EnrichedDecimalValueWidget : public EnrichedFormWidget
{
public:
    EnrichedDecimalValueWidget(QWidget *_parent = 0);
    void setPrecision(const quint8 &_precision);

protected:
    quint8 precision() const;
    QString withDecimalPrecision(QString _matrix_cell_value);
    virtual void applyPrecision() = 0;
private:
    quint8 m_precision;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_DECIMAL_VALUE_WIDGET_H_
