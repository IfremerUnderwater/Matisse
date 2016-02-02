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
    _table -> horizontalHeader() -> hide();
    _table -> verticalHeader() -> hide();
    _table->setColumnCount(cols);
    _table->setRowCount(rows);
    _table->horizontalScrollBar()->hide();
    _table->verticalScrollBar()->hide();
    _table->setFrameStyle(QFrame::NoFrame);

    for (int noCol=0; noCol < cols; noCol++) {
        _table->setColumnWidth(noCol, 70);
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

    _table->setMinimumSize(cols*70, rows*20);
    _table->setMaximumSize(cols*70, rows*20);

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

void EnrichedTableWidget::setValue(QString newValue)
{
    QStringList newValues = newValue.split(";");
    QStringList defaultValues = _defaultValue.split(";");

    if (newValues.size() != defaultValues.size()) {
        qWarning() << QString("New table values '%1' not matching initial size '%2', skipping...").arg(newValue).arg(_defaultValue);
        return;
    }

    int index = 0;
    for(int noRow = 0; noRow < _table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < _table->columnCount(); noCol++) {
            _table->setItem(noRow, noCol, new QTableWidgetItem(newValues.at(index)));
            index++;
        }
    }
}

void EnrichedTableWidget::restoreDefaultValue()
{
    QStringList defaultValues = _defaultValue.split(";");

    int index = 0;
    for(int noRow = 0; noRow < _table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < _table->columnCount(); noCol++) {
            _table->setItem(noRow, noCol, new QTableWidgetItem(defaultValues.at(index)));
            index++;
        }
    }
}

