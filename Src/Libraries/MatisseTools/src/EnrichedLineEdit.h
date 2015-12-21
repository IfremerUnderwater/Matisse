#ifndef ENRICHEDLINEEDIT_H
#define ENRICHEDLINEEDIT_H

#include <QLineEdit>
#include <QString>

#include "EnrichedFormWidget.h"

namespace MatisseTools {

class EnrichedLineEdit : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedLineEdit(QWidget *parent = 0, QString label = "", QString defaultValue = "");
    bool currentValueChanged();
    virtual QString currentValue();

signals:

public slots:
//    void slot_valueChanged();

private:
    QLineEdit * _lineEdit;

};

}

#endif // ENRICHEDLINEEDIT_H
