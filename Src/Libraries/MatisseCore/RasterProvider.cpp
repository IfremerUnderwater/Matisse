#include "RasterProvider.h"

using namespace MatisseCommon;

RasterProvider::RasterProvider(QObject *parent, QString name, QString comment, quint16 inNumber) :
    QObject(parent),
    LifecycleComponent(name, "[DESTINATION " + name + "]: "),
    _comment(comment),
    _inNumber(inNumber),
    _imageSet(NULL)
{
}

RasterProvider::~RasterProvider()
{
}


ImageSet *RasterProvider::imageSet() const
{
    return _imageSet;
}

void RasterProvider::setImageSet(ImageSet *imageSet)
{
    _imageSet = imageSet;
}
