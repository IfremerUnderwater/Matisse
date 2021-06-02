#ifndef MATISSE_ENRICHED_CHECK_BOX_H_
#define MATISSE_ENRICHED_CHECK_BOX_H_

#include <QtCore>
#include <QCheckBox>
#include <QString>

#include "enriched_form_widget.h"

namespace matisse {

class EnrichedCheckBox : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedCheckBox(QWidget *parent, QString label, bool checked);
//    bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

public slots:
//    void slot_valueChanged();

private:
    QCheckBox * _check;

};

} // namespace matisse

#endif // MATISSE_ENRICHED_CHECK_BOX_H_
