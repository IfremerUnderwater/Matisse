#ifndef MATISSE_ENRICHED_SPIN_BOX_H_
#define MATISSE_ENRICHED_SPIN_BOX_H_

#include <QSpinBox>
#include <QString>

#include "enriched_form_widget.h"

#define MIN_SINT32 -2147483647
#define MAX_SINT32 2147483647

namespace matisse {

class EnrichedSpinBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue);
//    bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

public slots:

private:
    QSpinBox * _spin;
    qint32 _minValueInt;
    qint32 _maxValueInt;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_SPIN_BOX_H_
