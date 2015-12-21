#include "ParametersWidgetSkeleton.h"
#include "ui_ParametersWidgetSkeleton.h"

#include <QtDebug>

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

void ParametersWidgetSkeleton::clearWidget()
{
    foreach(QWidget * widget, _widgets) {
        _ui->_VLAY_parameters->removeWidget(widget);
        widget->deleteLater();
    }
    _modifiedValues.clear();
}

bool ParametersWidgetSkeleton::hasModifiedValues()
{
    return (_modifiedValues.size() > 0);
}

void ParametersWidgetSkeleton::slot_valueModified(bool trueOrFalse)
{
    QObject * widSender = sender();
    if (trueOrFalse == false) {
        _modifiedValues.removeOne(widSender);
    } else if (!_modifiedValues.contains(widSender)) {
        _modifiedValues.append(widSender);
    }
    emit signal_valuesModified(hasModifiedValues());
    qDebug() << "Emit signal modified 1";
}
