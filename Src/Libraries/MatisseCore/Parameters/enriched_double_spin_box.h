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
    explicit EnrichedDoubleSpinBox(QWidget *_parent, QString m_label, QString _min_value, QString _max_value, QString _default_value);
    //bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);
    virtual void applyPrecision();

signals:

public slots:

private:
    QDoubleSpinBox * m_spin;

    double m_min_value_real;
    double m_max_value_real;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_DOUBLE_SPIN_BOX_H_
