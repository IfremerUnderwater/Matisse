#include "lifecycle_component.h"
#include <QDir>
#include "file_utils.h"

namespace matisse {

LifecycleComponent::LifecycleComponent(QString _name, QString _log_prefix) :
    m_name(_name),
    m_log_prefix(_log_prefix)
{
}

QString LifecycleComponent::name() const
{
    return m_name;
}

QList<MatisseParameter> LifecycleComponent::expectedParameters() const
{
    return m_expected_parameters;
}

void LifecycleComponent::addExpectedParameter(QString _structure, QString _param)
{
    MatisseParameter parameter;
    parameter.m_structure = _structure;
    parameter.m_param = _param;

    m_expected_parameters.append(parameter);
}

QString LifecycleComponent::logPrefix() const
{
    return m_log_prefix;
}

bool LifecycleComponent::isStarted() const
{
    return m_is_started;
}

bool LifecycleComponent::isCancelled() const
{
    return m_is_cancelled;
}


bool LifecycleComponent::callConfigure(Context * _context, MatisseParameters * _matisse_parameters)
{
    qDebug() << logPrefix() << "configure";
    m_context = _context;
    m_matisse_parameters = _matisse_parameters;
    return configure();
}

bool LifecycleComponent::callStart()
{
    qDebug() << logPrefix() << "start";
    m_is_started = true;
    return start();
}

bool LifecycleComponent::askToStop(bool _cancel)
{
    qDebug() << logPrefix() << "askForStop";
    m_is_started = false;
    m_is_cancelled = _cancel;
    return true;
}

bool LifecycleComponent::callStop()
{
    qDebug() << logPrefix() << "stop";
    m_is_started = false;
    return stop();
}

QString LifecycleComponent::absoluteDatasetDir()
{
    return system_tools::FileUtils::resolveUnixPath(m_matisse_parameters->getStringParamValue("dataset_param", "dataset_dir"));
}

QString LifecycleComponent::absoluteOutputDir()
{
    QDir output_dir(system_tools::FileUtils::resolveUnixPath(m_matisse_parameters->getStringParamValue("dataset_param", "output_dir")));
    
    if (output_dir.isRelative())
    {
        QString dataset_dir = this->absoluteDatasetDir();
        return dataset_dir + QDir::separator() + output_dir.path();
    }
    else
        return output_dir.path();

}

QString LifecycleComponent::absoluteOutputTempDir()
{
    QDir temp_path(this->absoluteOutputDir() + QDir::separator() + "temp");

    if (!temp_path.exists())
    {
        temp_path.mkpath(temp_path.path());
    }

    return temp_path.path();
}

} // namespace matisse

