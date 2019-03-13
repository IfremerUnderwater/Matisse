#ifndef ENRICHEDTABLEWIDGET_H
#define ENRICHEDTABLEWIDGET_H

#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QtDebug>

#include "EnrichedDecimalValueWidget.h"

namespace MatisseTools {

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
}

#endif // ENRICHEDTABLEWIDGET_H
