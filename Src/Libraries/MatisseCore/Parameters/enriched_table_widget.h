#ifndef MATISSE_ENRICHED_TABLE_WIDGET_H_
#define MATISSE_ENRICHED_TABLE_WIDGET_H_

#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QtDebug>

#include "enriched_decimal_value_widget.h"

namespace matisse {

class EnrichedTableWidget : public EnrichedDecimalValueWidget
{
    Q_OBJECT
public:
    explicit EnrichedTableWidget(QWidget *parent, QString label, quint8 cols, quint8 rows, QStringList defaultValues, QString formatTemplate);
//    bool currentValueChanged();
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);
    virtual void applyPrecision();

signals:

protected slots:
    void slot_cellValueChanged(QTableWidgetItem *item);

private:
    QTableWidget * _table;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_TABLE_WIDGET_H_
