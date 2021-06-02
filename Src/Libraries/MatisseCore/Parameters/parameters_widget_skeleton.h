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
    explicit ParametersWidgetSkeleton(QWidget *parent = 0);
    ~ParametersWidgetSkeleton();

    void addWidget(QWidget * widget);
    void clearModifications();
    bool hasModifiedValues();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

protected slots:
    void slot_valueModified(bool trueOrFalse);

signals:
    void signal_valuesModified(bool trueOrFalse);
    void signal_translateParameters();

private:
    Ui::ParametersWidgetSkeleton *_ui;
    QList<QWidget*> _widgets;
    QList<QObject*>_modifiedValues;
};

} // namespace matisse

#endif // MATISSE_PARAMETERS_WIDGET_SKELETON_H_
