#ifndef ENRICHEDFORMWIDGET_H
#define ENRICHEDFORMWIDGET_H

#include <QWidget>
#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QSharedPointer>

#include "GraphicalCharter.h"

#include <limits>

namespace MatisseTools {
class EnrichedFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnrichedFormWidget(QWidget *parent = 0);

    virtual QString currentValue() =0;
    virtual void restoreDefaultValue() = 0;
    virtual void setValue(QString newValue) = 0;

protected:
    void setWidget(QString label, QWidget * widget, bool wrapWidget = false);
    virtual bool currentValueChanged() = 0;
    void swapColor(bool yes = true);
    quint32 getTextFieldWidth(QString text);

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
