#ifndef PARAMETERSWIDGETSKELETON_H
#define PARAMETERSWIDGETSKELETON_H

#include <QWidget>
#include <QGroupBox>
#include "EnrichedFormWidget.h"

namespace Ui {
class ParametersWidgetSkeleton;
}

class ParametersWidgetSkeleton : public QWidget
{
    Q_OBJECT

public:
    explicit ParametersWidgetSkeleton(QWidget *parent = 0);
    ~ParametersWidgetSkeleton();

    void addWidget(QWidget * widget);
    void clearWidget();
    bool hasModifiedValues();

protected slots:
    void slot_valueModified(bool trueOrFalse);

signals:
    void signal_valuesModified(bool trueOrFalse);

private:
    Ui::ParametersWidgetSkeleton *_ui;
    QList<QWidget*> _widgets;
    QList<QObject*>_modifiedValues;
};

#endif // PARAMETERSWIDGETSKELETON_H
