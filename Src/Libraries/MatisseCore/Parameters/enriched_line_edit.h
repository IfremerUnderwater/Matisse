#ifndef MATISSE_ENRICHED_LINE_EDIT_H_
#define MATISSE_ENRICHED_LINE_EDIT_H_

#include <QLineEdit>
#include <QString>

#include "enriched_form_widget.h"

namespace matisse {

class EnrichedLineEdit : public EnrichedFormWidget
{
    Q_OBJECT
public:
    explicit EnrichedLineEdit(QWidget *parent = 0, QString label = "", QString defaultValue = "");
    //bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);

signals:

public slots:
//    void slot_valueChanged();

private:
    QLineEdit * _lineEdit;

};

} // namespace matisse

#endif // MATISSE_ENRICHED_LINE_EDIT_H_
