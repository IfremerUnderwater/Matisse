#ifndef MATISSE_PARAMETERS_WIDGET_SKELETON_H_
#define MATISSE_PARAMETERS_WIDGET_SKELETON_H_

#include <QWidget>
#include <QGroupBox>
#include "enriched_form_widget.h"

namespace Ui {
class ParametersWidgetSkeleton;
}

namespace matisse {

class ParametersWidgetSkeleton : public QWidget
{
    Q_OBJECT
public:
    explicit ParametersWidgetSkeleton(QWidget *_parent = 0);
    ~ParametersWidgetSkeleton();

    void addWidget(QWidget * _widget);
    void clearModifications();
    bool hasModifiedValues();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

protected slots:
    void sl_valueModified(bool _has_changed);

signals:
    void si_valuesModified(bool _has_changed);
    void si_translateParameters();

private:
    Ui::ParametersWidgetSkeleton *m_ui;
    QList<QWidget*> m_widgets;
    QList<QObject*>m_modified_values;
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_WIDGET_SKELETON_H_
