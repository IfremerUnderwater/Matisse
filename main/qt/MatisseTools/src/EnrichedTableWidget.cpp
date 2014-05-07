#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>

#include "EnrichedTableWidget.h"

using namespace MatisseTools;

EnrichedTableWidget::EnrichedTableWidget(QWidget *parent, QString label, quint8 cols, quint8 rows, QStringList defaultValues) :
    EnrichedFormWidget(parent)
{
    _table = new QTableWidget(this);
    _table -> horizontalHeader() -> setVisible(false);
    _table -> verticalHeader() -> setVisible(false);
    _table->setColumnCount(cols);
    _table->setRowCount(rows);
    _table->horizontalScrollBar()->hide();
    _table->verticalScrollBar()->hide();
    _table->setFrameStyle(QFrame::NoFrame);

    for (int noCol=0; noCol < cols; noCol++) {
        _table->setColumnWidth(noCol, 45);
    }
    for (int noRow=0; noRow < rows; noRow++) {
        _table->setRowHeight(noRow, 20);
    }

    int index = 0;
    for(int noRow = 0; noRow < rows; noRow++) {
        for (int noCol = 0; noCol < cols; noCol++) {
            _table->setItem(noRow, noCol, new QTableWidgetItem(defaultValues.at(index)));
            index++;
        }
    }

    _table->setMinimumSize(cols*45, rows*20);
    _table->setMaximumSize(cols*45, rows*20);

    _defaultValue = defaultValues.join(";");

    setWidget(label, _table);

    connect(_table, SIGNAL(cellChanged(int,int)), this, SLOT(slot_valueChanged()));

}

bool EnrichedTableWidget::currentValueChanged()
{
    return currentValue() != _defaultValue;
}

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

