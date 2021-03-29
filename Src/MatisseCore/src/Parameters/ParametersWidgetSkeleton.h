#ifndef PARAMETERSWIDGETSKELETON_H
#define PARAMETERSWIDGETSKELETON_H

#include <QWidget>
#include <QGroupBox>
#include "EnrichedFormWidget.h"

namespace Ui {
class ParametersWidgetSkeleton;
}

namespace MatisseTools {

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

}

#endif // PARAMETERSWIDGETSKELETON_H
