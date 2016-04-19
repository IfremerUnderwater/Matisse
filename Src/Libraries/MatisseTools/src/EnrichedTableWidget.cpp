#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>

#include "EnrichedTableWidget.h"

using namespace MatisseTools;

EnrichedTableWidget::EnrichedTableWidget(QWidget *parent, QString label, quint8 cols, quint8 rows, QStringList defaultValues, QString formatTemplate) :
    EnrichedDecimalValueWidget(parent)
{
    quint32 colWidth = PARAM_TABLE_COL_WIDTH_MAX;
    if (!formatTemplate.isEmpty()) {
        colWidth = getTextFieldWidth(formatTemplate) + PARAM_TABLE_CELL_PADDING;
    }

    _table = new QTableWidget(this);
    _table -> horizontalHeader() -> hide();
    _table -> verticalHeader() -> hide();
    _table->setColumnCount(cols);
    _table->setRowCount(rows);
    _table->horizontalScrollBar()->hide();
    _table->verticalScrollBar()->hide();
    _table->setFrameStyle(QFrame::NoFrame);

    for (int noCol=0; noCol < cols; noCol++) {
        _table->setColumnWidth(noCol, colWidth);
    }
    for (int noRow=0; noRow < rows; noRow++) {
        _table->setRowHeight(noRow, PARAM_TABLE_ROW_HEIGHT);
    }

    int index = 0;
    for(int noRow = 0; noRow < rows; noRow++) {
        for (int noCol = 0; noCol < cols; noCol++) {
            _table->setItem(noRow, noCol, new QTableWidgetItem(defaultValues.at(index)));
            index++;
        }
    }

    quint16 tableWidth = cols * colWidth;
    quint16 tableHeight = rows * PARAM_TABLE_ROW_HEIGHT;


    _table->setMinimumSize(tableWidth, tableHeight);
    _table->setMaximumSize(tableWidth, tableHeight);

    _defaultValue = defaultValues.join(";");

    bool widgetWrap = tableWidth > PARAM_TABLE_WIDTH_NOWRAP_MAX;
    setWidget(label, _table, widgetWrap);

    connect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));
    connect(_table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(slot_cellValueChanged(QTableWidgetItem*)));
}

//bool EnrichedTableWidget::currentValueChanged()
//{
//    return currentValue() != _defaultValue;
//}

QString EnrichedTableWidget::currentValue()
{
    QStringList currentValues;
    for(int noRow = 0; noRow < _table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < _table->columnCount(); noCol++) {
            currentValues << _table->item(noRow, noCol)->text().trimmed();
        }
    }
    return currentValues.join(";");
}

void EnrichedTableWidget::applyValue(QString newValue)
{
    disconnect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));

    QStringList newValues = newValue.split(";");
    QStringList defaultValues = _defaultValue.split(";");

    if (newValues.size() != defaultValues.size()) {
        qWarning() << QString("New table values '%1' not matching initial size '%2', skipping...").arg(newValue).arg(_defaultValue);
        return;
    }

    int index = 0;
    for(int noRow = 0; noRow < _table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < _table->columnCount(); noCol++) {
            QString cellValueWithPrecision = withDecimalPrecision(newValues.at(index));
            _table->setItem(noRow, noCol, new QTableWidgetItem(cellValueWithPrecision));
            index++;
        }
    }

    connect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));
}

void EnrichedTableWidget::applyPrecision()
{
    // NOTHING
}

void EnrichedTableWidget::slot_cellValueChanged(QTableWidgetItem* item)
{
    QString itemText = item->text();
    QString cellValueWithPrecision = withDecimalPrecision(itemText);
    item->setText(cellValueWithPrecision);
}

void EnrichedTableWidget::restoreDefaultValue()
{
    disconnect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));

    QStringList defaultValues = _defaultValue.split(";");

    int index = 0;
    for(int noRow = 0; noRow < _table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < _table->columnCount(); noCol++) {
            QString cellValueWithPrecision = withDecimalPrecision(defaultValues.at(index));
            _table->setItem(noRow, noCol, new QTableWidgetItem(cellValueWithPrecision));
            index++;
        }
    }

    connect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));
}

