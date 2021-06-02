#include "parameters_widget_skeleton.h"
#include "ui_parameters_widget_skeleton.h"

#include <QtDebug>

namespace matisse {

ParametersWidgetSkeleton::ParametersWidgetSkeleton(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::ParametersWidgetSkeleton)
{
    _ui->setupUi(this);
}

ParametersWidgetSkeleton::~ParametersWidgetSkeleton()
{
    delete _ui;
}

void ParametersWidgetSkeleton::addWidget(QWidget *widget)
{
    _ui->_VLAY_parameters->addWidget(widget);
    _widgets << widget;
}

void ParametersWidgetSkeleton::clearModifications()
{
    _modifiedValues.clear();
}

bool ParametersWidgetSkeleton::hasModifiedValues()
{
    return (_modifiedValues.size() > 0);
}

void ParametersWidgetSkeleton::slot_valueModified(bool isModified)
{
    QObject * widSender = sender();
    if (!isModified) {
        _modifiedValues.removeOne(widSender);
    } else if (!_modifiedValues.contains(widSender)) {
        _modifiedValues.append(widSender);
    }
    emit signal_valuesModified(hasModifiedValues());
}

// Dynamic translation
void ParametersWidgetSkeleton::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form
        qDebug() << "Parameters widget UI retranslation...";
        emit signal_translateParameters();
    }
}

} // namespace matisse
