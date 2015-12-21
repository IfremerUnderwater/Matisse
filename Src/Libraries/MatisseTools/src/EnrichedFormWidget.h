#ifndef ENRICHEDFORMWIDGET_H
#define ENRICHEDFORMWIDGET_H

#include <QWidget>
#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QSharedPointer>

#include <limits>

namespace MatisseTools {
class EnrichedFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnrichedFormWidget(QWidget *parent = 0);

    void setLabelColumnWidth(quint32 width);
    quint32 getLabelColumnWidth();
    void setWidth(quint32 labelWidth, quint32 widgetWidth);
    virtual QString currentValue() =0;

protected:
    void setWidget(QString label, QWidget * widget);
    quint32 getLabelWidth();
    virtual bool currentValueChanged() = 0;
    void swapColor(bool yes = true);

    QString _defaultValue;


private:
    QGridLayout * _gridLayout;
    QLabel * _label;
    QWidget * _widget;

    static QSharedPointer<QFontMetrics> _metrics;

signals:
    void signal_valueChanged(bool trueOrFalse);

public slots:
    void slot_valueChanged();

};
}
#endif // ENRICHEDFORMWIDGET_H
