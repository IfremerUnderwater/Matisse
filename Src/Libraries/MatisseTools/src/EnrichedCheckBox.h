#ifndef ENRICHEDCHECKBOX_H
#define ENRICHEDCHECKBOX_H

#include <QtCore>
#include <QCheckBox>
#include <QString>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

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
}

#endif // ENRICHEDCHECKBOX_H
