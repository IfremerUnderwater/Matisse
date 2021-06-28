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
    explicit EnrichedTableWidget(QWidget *_parent, QString _label, quint8 _cols, quint8 _rows, QStringList _default_values, QString _format_template);
    virtual QString currentValue();
    virtual void restoreDefaultValue();

protected:
    virtual void applyValue(QString newValue);
    virtual void applyPrecision();

signals:

protected slots:
    void sl_cellValueChanged(QTableWidgetItem *item);

private:
    QTableWidget * m_table;
};

} // namespace matisse

#endif // MATISSE_ENRICHED_TABLE_WIDGET_H_
