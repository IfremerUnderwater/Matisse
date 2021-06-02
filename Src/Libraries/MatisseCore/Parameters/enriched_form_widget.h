#ifndef MATISSE_ENRICHED_FORM_WIDGET_H_
#define MATISSE_ENRICHED_FORM_WIDGET_H_

#include <QWidget>
#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QSharedPointer>

#include "graphical_charter.h"

#include <limits>

namespace matisse {

class EnrichedFormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnrichedFormWidget(QWidget *parent = 0);

    virtual QString currentValue() =0;
    virtual void restoreDefaultValue() = 0;
    void setValue(QString newValue);

    void setLabelText(QString text) {
        if (_label) {
            _label->setText(text);
        }
    }

    void overrideDefaultValue(QString newDefaultValue) {
        _defaultValue = newDefaultValue;
    }

protected:
    virtual void applyValue(QString newValue) = 0;
    void setWidget(QString label, QWidget * widget, bool wrapWidget = false);
    virtual bool currentValueChanged();
    void swapColor(bool yes = true);
    quint32 getTextFieldWidth(QString text);

    QString _defaultValue; // default value defined for the parameter in the dictionnary
    QString _initialValue; // assembly or job parameter value before user modification


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

} // namespace matisse

#endif // MATISSE_ENRICHED_FORM_WIDGET_H_
