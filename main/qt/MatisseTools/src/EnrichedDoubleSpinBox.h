#ifndef ENRICHEDDOUBLESPINBOX_H
#define ENRICHEDDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QString>

#include "EnrichedFormWidget.h"

#define MIN_REAL std::numeric_limits<float>::min()
#define MAX_REAL std::numeric_limits<float>::max()

namespace MatisseTools {

class EnrichedDoubleSpinBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedDoubleSpinBox(QWidget *parent, QString label, QString minValue, QString maxValue, QString defaultValue);
    bool currentValueChanged();
    virtual QString currentValue();

signals:

public slots:

private:
    QDoubleSpinBox * _spin;

};
}

#endif // ENRICHEDDOUBLESPINBOX_H
