#include "PlatformComparisonStatus.h"

using namespace MatisseTools;

PlatformComparisonStatus::PlatformComparisonStatus() :
    _componentsCompared(NULL),
    _doesMatch(false)
{
}

PlatformComparisonStatus::~PlatformComparisonStatus()
{
    if (_componentsCompared) {
        _componentsCompared->clear();
        delete _componentsCompared;
    }
}

bool PlatformComparisonStatus::doesMatch() const
{
    return _doesMatch;
}

void PlatformComparisonStatus::setDoesMatch(bool doesMatch)
{
    _doesMatch = doesMatch;
}

PlatformElementCompare PlatformComparisonStatus::matisseCompared() const
{
    return _matisseCompared;
}

void PlatformComparisonStatus::setMatisseCompared(const PlatformElementCompare &matisseCompared)
{
    _matisseCompared = matisseCompared;
}

PlatformElementCompare PlatformComparisonStatus::osCompared() const
{
    return _osCompared;
}

void PlatformComparisonStatus::setOsCompared(const PlatformElementCompare &osCompared)
{
    _osCompared = osCompared;
}

QMap<QString, PlatformElementCompare> *PlatformComparisonStatus::componentsCompared() const
{
    return _componentsCompared;
}

void PlatformComparisonStatus::addComponentCompared(QString componentName, PlatformElementCompare compared)
{
    if (!_componentsCompared) {
        _componentsCompared = new QMap<QString, PlatformElementCompare>();
    }

    _componentsCompared->insert(componentName, compared);
}

QStringList PlatformComparisonStatus::getPlatformGapMessages() const
{
    return _platformGapMessages;
}

void PlatformComparisonStatus::addPlaformGapMessage(QString message)
{
    _platformGapMessages.append(message);
}






