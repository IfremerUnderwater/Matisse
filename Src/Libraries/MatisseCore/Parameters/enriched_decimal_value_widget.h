#ifndef ENRICHEDDECIMALVALUEWIDGET_H
#define ENRICHEDDECIMALVALUEWIDGET_H

#include "enriched_form_widget.h"

namespace MatisseTools {

/* Base class for enriched form classes displaying decimal values */
class EnrichedDecimalValueWidget : public EnrichedFormWidget
{
public:
    EnrichedDecimalValueWidget(QWidget *parent = 0);
    void setPrecision(const quint8 &precision);

protected:
    quint8 precision() const;
    QString withDecimalPrecision(QString matrixCellValue);
    virtual void applyPrecision() = 0;
private:
    quint8 _precision;
};

}

#endif // ENRICHEDDECIMALVALUEWIDGET_H
