#include "lifecycle_component.h"
#include <QDir>

namespace matisse {

LifecycleComponent::LifecycleComponent(QString name, QString logPrefix)
: _name(name),
  _logPrefix(logPrefix)
{
}

QString LifecycleComponent::name() const
{
    return _name;
}

QList<MatisseParameter> LifecycleComponent::expectedParameters() const
{
    return _expectedParameters;
}

void LifecycleComponent::addExpectedParameter(QString structure, QString param)
{
    MatisseParameter parameter;
    parameter.structure = structure;
    parameter.param = param;

    _expectedParameters.append(parameter);
}

QString LifecycleComponent::logPrefix() const
{
    return _logPrefix;
}

bool LifecycleComponent::isStarted() const
{
    return _isStarted;
}

bool LifecycleComponent::isCancelled() const
{
    return _isCancelled;
}


bool LifecycleComponent::callConfigure(Context * context, MatisseParameters * matisseParameters)
{
    qDebug() << logPrefix() << "configure";
    _context = context;
    _matisseParameters = matisseParameters;
    return configure();
}

bool LifecycleComponent::callStart()
{
    qDebug() << logPrefix() << "start";
    _isStarted = true;
    return start();
}

bool LifecycleComponent::askToStop(bool cancel)
{
    qDebug() << logPrefix() << "askForStop";
    _isStarted = false;
    _isCancelled = cancel;
    return true;
}

bool LifecycleComponent::callStop()
{
    qDebug() << logPrefix() << "stop";
    _isStarted = false;
    return stop();
}

QString LifecycleComponent::absoluteDatasetDir()
{
    return _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
}

QString LifecycleComponent::absoluteOutputDir()
{
    QDir output_dir(_matisseParameters->getStringParamValue("dataset_param", "output_dir"));
    
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

