#include "LifecycleComponent.h"

using namespace MatisseCommon;

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

