#include <QWidget>
#include <QString>
#include <QStringList>
#include <QTableWidget>
#include <QHeaderView>
#include <QScrollBar>

#include "enriched_table_widget.h"
#include "graphical_charter.h"

namespace matisse {

EnrichedTableWidget::EnrichedTableWidget(QWidget *_parent, QString _label, quint8 _cols, quint8 _rows, QStringList _default_values, QString _format_template) :
    EnrichedDecimalValueWidget(_parent)
{
    GraphicalCharter &graph_chart = GraphicalCharter::instance();
    quint32 col_width = graph_chart.dpiScaled(PARAM_TABLE_COL_WIDTH_MAX);
    if (!_format_template.isEmpty()) {
        col_width = getTextFieldWidth(_format_template) + graph_chart.dpiScaled(PARAM_TABLE_CELL_PADDING);
    }

    m_table = new QTableWidget(this);
    m_table -> horizontalHeader() -> hide();
    m_table -> verticalHeader() -> hide();
    m_table->setColumnCount(_cols);
    m_table->setRowCount(_rows);
    m_table->horizontalScrollBar()->hide();
    m_table->verticalScrollBar()->hide();
    m_table->setFrameStyle(QFrame::NoFrame);

    for (int no_col=0; no_col < _cols; no_col++) {
        m_table->setColumnWidth(no_col, col_width);
    }
    for (int no_row=0; no_row < _rows; no_row++) {
        m_table->setRowHeight(no_row, graph_chart.dpiScaled(PARAM_TABLE_ROW_HEIGHT));
    }

    int index = 0;
    for(int no_row = 0; no_row < _rows; no_row++) {
        for (int noCol = 0; noCol < _cols; noCol++) {
            m_table->setItem(no_row, noCol, new QTableWidgetItem(_default_values.at(index)));
            index++;
        }
    }

    quint16 table_width = _cols * col_width;
    quint16 table_height = _rows * graph_chart.dpiScaled(PARAM_TABLE_ROW_HEIGHT);

    m_table->setFixedSize(table_width+graph_chart.dpiScaled(5), table_height+graph_chart.dpiScaled(10));

    m_default_value = _default_values.join(";");

    bool widget_wrap = table_width > graph_chart.dpiScaled(PARAM_TABLE_WIDTH_NOWRAP_MAX);
    setWidget(_label, m_table, widget_wrap);

    connect(m_table, SIGNAL(cellChanged(int,int)), this, SLOT(sl_valueChanged()));
    connect(m_table, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(sl_cellValueChanged(QTableWidgetItem*)));
}

QString EnrichedTableWidget::currentValue()
{
    QStringList current_values;
    for(int noRow = 0; noRow < m_table->rowCount(); noRow++) {
        for (int noCol = 0; noCol < m_table->columnCount(); noCol++) {
            current_values << m_table->item(noRow, noCol)->text().trimmed();
        }
    }
    return current_values.join(";");
}

void EnrichedTableWidget::applyValue(QString newValue)
{
    disconnect(m_table, SIGNAL(cellChanged(int,int)), this, SLOT(sl_valueChanged()));

    QStringList new_values = newValue.split(";");
    QStringList default_values = m_default_value.split(";");

    if (new_values.size() != default_values.size()) {
        qWarning() << QString("New table values '%1' not matching initial size '%2', skipping...").arg(newValue).arg(m_default_value);
        return;
    }

    int index = 0;
    for(int no_row = 0; no_row < m_table->rowCount(); no_row++) {
        for (int no_col = 0; no_col < m_table->columnCount(); no_col++) {
            QString cell_value_with_precision = withDecimalPrecision(new_values.at(index));
            m_table->setItem(no_row, no_col, new QTableWidgetItem(cell_value_with_precision));
            index++;
        }
    }

    connect(m_table, SIGNAL(cellChanged(int,int)), this, SLOT(sl_valueChanged()));
}

void EnrichedTableWidget::applyPrecision()
{
    // NOTHING
}

void EnrichedTableWidget::sl_cellValueChanged(QTableWidgetItem* item)
{
    QString item_text = item->text();
    QString cell_value_with_precision = withDecimalPrecision(item_text);
    item->setText(cell_value_with_precision);
}

void EnrichedTableWidget::restoreDefaultValue()
{
    disconnect(m_table, SIGNAL(cellChanged(int,int)), this, SLOT(sl_valueChanged()));

    QStringList default_values = m_default_value.split(";");

    int index = 0;
    for(int no_row = 0; no_row < m_table->rowCount(); no_row++) {
        for (int no_col = 0; no_col < m_table->columnCount(); no_col++) {
            QString cell_value_with_precision = withDecimalPrecision(default_values.at(index));
            m_table->setItem(no_row, no_col, new QTableWidgetItem(cell_value_with_precision));
            index++;
        }
    }

    connect(m_table, SIGNAL(cellChanged(int,int)), this, SLOT(sl_valueChanged()));
}

} // namespace matisse

