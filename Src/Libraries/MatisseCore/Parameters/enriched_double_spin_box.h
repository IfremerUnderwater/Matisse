#ifndef MATISSE_ENRICHED_DOUBLE_SPIN_BOX_H_
#define MATISSE_ENRICHED_DOUBLE_SPIN_BOX_H_

#include <QDoubleSpinBox>
#include <QString>
#include <QtDebug>
#include <qmath.h>

#include "parameters_common.h"
#include "enriched_decimal_value_widget.h"

#define MIN_REAL std::numeric_limits<float>::min()
#define MAX_REAL std::numeric_limits<float>::max()

namespace matisse {

class EnrichedDoubleSpinBox : public EnrichedDecimalValueWidget
{
    Q_OBJECT
public:
    explicit EnrichedDoubleSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue);
    //bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);
    virtual void applyPrecision();

signals:

public slots:

private:
    QDoubleSpinBox * _spin;

    double _minValueReal;
    double _maxValueReal;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_DOUBLE_SPIN_BOX_H_
