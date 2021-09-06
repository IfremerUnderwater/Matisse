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
    explicit EnrichedFormWidget(QWidget *_parent = 0);

    virtual QString currentValue() =0;
    virtual void restoreDefaultValue() = 0;
    void setValue(QString _new_value);

    void setLabelText(QString _text) {
        if (m_label) {
            m_label->setText(_text);
        }
    }

    void overrideDefaultValue(QString _new_default_value) {
        m_default_value = _new_default_value;
    }

protected:
    virtual void applyValue(QString _new_value) = 0;
    void setWidget(QString _label, QWidget * _widget, bool _wrap_widget = false);
    virtual bool currentValueChanged();
    void swapColor(bool _do_swap = true);
    quint32 getTextFieldWidth(QString _text);

    QString m_default_value; // default value defined for the parameter in the dictionnary
    QString m_initial_value; // assembly or job parameter value before user modification


private:
    QGridLayout * m_grid_layout;
    QLabel * m_label;
    QWidget * m_widget;

    static QSharedPointer<QFontMetrics> m_metrics;

signals:
    void si_valueChanged(bool _has_changed);

public slots:
    void sl_valueChanged();

};

} // namespace matisse

#endif // MATISSE_ENRICHED_FORM_WIDGET_H_
