#include "input_data_provider.h"

using namespace MatisseCommon;

InputDataProvider::InputDataProvider(QObject *parent, QString name, QString comment, quint16 outNumber) :
    QObject(parent),
    LifecycleComponent(name, "[SOURCE " + name + "]: "),
    _comment(comment),
    _outNumber(outNumber),
    _isRealTime(false)
{
}

InputDataProvider::~InputDataProvider()
{

}


QString InputDataProvider::comment() const
{
    return _comment;
}
bool InputDataProvider::isRealTime() const
{
    return _isRealTime;
}

void InputDataProvider::setIsRealTime(bool isRealTime)
{
    _isRealTime = isRealTime;
}




