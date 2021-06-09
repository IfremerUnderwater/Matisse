#include "parameters_widget_skeleton.h"
#include "ui_parameters_widget_skeleton.h"

#include <QtDebug>

namespace matisse {

ParametersWidgetSkeleton::ParametersWidgetSkeleton(QWidget *_parent) :
    QWidget(_parent),
    m_ui(new Ui::ParametersWidgetSkeleton)
{
    m_ui->setupUi(this);
}

ParametersWidgetSkeleton::~ParametersWidgetSkeleton()
{
    delete m_ui;
}

void ParametersWidgetSkeleton::addWidget(QWidget *_widget)
{
    m_ui->_VLAY_parameters->addWidget(_widget);
    m_widgets << _widget;
}

void ParametersWidgetSkeleton::clearModifications()
{
    m_modified_values.clear();
}

bool ParametersWidgetSkeleton::hasModifiedValues()
{
    return (m_modified_values.size() > 0);
}

void ParametersWidgetSkeleton::sl_valueModified(bool _has_changed)
{
    QObject * wid_sender = sender();
    if (!_has_changed) {
        m_modified_values.removeOne(wid_sender);
    } else if (!m_modified_values.contains(wid_sender)) {
        m_modified_values.append(wid_sender);
    }
    emit si_valuesModified(hasModifiedValues());
}

// Dynamic translation
void ParametersWidgetSkeleton::changeEvent(QEvent *_event)
{
    if (_event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form
        qDebug() << "Parameters widget UI retranslation...";
        emit si_translateParameters();
    }
}

} // namespace matisse
