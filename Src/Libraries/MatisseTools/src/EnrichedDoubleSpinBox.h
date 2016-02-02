#ifndef ENRICHEDDOUBLESPINBOX_H
#define ENRICHEDDOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QString>
#include <QtDebug>

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
    virtual void restoreDefaultValue();
    virtual void setValue(QString newValue);

signals:

public slots:

private:
    QDoubleSpinBox * _spin;

    qreal _minValueReal;
    qreal _maxValueReal;
};
}

#endif // ENRICHEDDOUBLESPINBOX_H
