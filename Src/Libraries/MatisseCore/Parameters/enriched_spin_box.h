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
    explicit EnrichedSpinBox(QWidget *_parent, QString _label, QString _min_value, QString _max_value, QString _default_value);
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString _new_value);

signals:

public slots:

private:
    QSpinBox * m_spin;
    qint32 m_min_value_int;
    qint32 m_max_value_int;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_SPIN_BOX_H_
