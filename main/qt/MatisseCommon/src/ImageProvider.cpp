#include "ImageProvider.h"

using namespace MatisseCommon;

ImageProvider::ImageProvider(QObject *parent, QString name, QString comment, quint16 outNumber) :
    QObject(parent),
    LifecycleComponent(name, "[SOURCE " + name + "]: "),
    _comment(comment),
    _outNumber(outNumber),
    _isRealTime(false)
{
}

ImageProvider::~ImageProvider()
{

}


QString ImageProvider::comment() const
{
    return _comment;
}
bool ImageProvider::isRealTime() const
{
    return _isRealTime;
}

void ImageProvider::setIsRealTime(bool isRealTime)
{
    _isRealTime = isRealTime;
}




