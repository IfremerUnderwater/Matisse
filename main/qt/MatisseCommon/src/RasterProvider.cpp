#include "RasterProvider.h"

using namespace MatisseCommon;

RasterProvider::RasterProvider(QObject *parent, QString name, QString comment, quint16 inNumber) :
    ImageListener(parent),
    _name(name),
    _comment(comment),
    _inNumber(inNumber),
    _imageSet(NULL)
{
    _logPrefix = "[DESTINATION " + _name + "]: ";
}

RasterProvider::~RasterProvider()
{

}

void RasterProvider::addExpectedParameter(QString structure, QString param)
{
    MatisseParameter parameter;
    parameter.structure = structure;
    parameter.param = param;
    _expectedParameters.append(parameter);

}

ImageSet *RasterProvider::imageSet() const
{
return _imageSet;
}

void RasterProvider::setImageSet(ImageSet *imageSet)
{
_imageSet = imageSet;
}


QString const &RasterProvider::logPrefix() const
{
    return  _logPrefix;
}


void RasterProvider::callConfigure(Context * context, MatisseParameters * mosaicParameters)
{
    qDebug() << logPrefix() << "configure";
    _context = context;
    _mosaicParameters = mosaicParameters;
    configure(context, mosaicParameters);
}


void RasterProvider::callStart()
{
    qDebug() << logPrefix() << "start";
    start();
}

void RasterProvider::callStop()
{
    qDebug() << logPrefix() << "stop";
    stop();
}
