#include "ImageProvider.h"

using namespace MatisseCommon;

ImageProvider::ImageProvider(QObject *parent, QString name, QString comment, quint16 outNumber) :
    QObject(parent),
    _name(name),
    _comment(comment),
    _outNumber(outNumber)
{
    _logPrefix = "[SOURCE " + _name + "]: ";
}

ImageProvider::~ImageProvider()
{

}

QString ImageProvider::name() const
{
    return _name;
}
QString ImageProvider::comment() const
{
    return _comment;
}

void ImageProvider::addExpectedParameter(QString structure, QString param)
{
    MatisseParameter parameter;
    parameter.structure = structure;
    parameter.param = param;

    _expectedParameters.append(parameter);
}

QString const& ImageProvider::logPrefix() const
{
    return _logPrefix;
}

bool ImageProvider::callConfigure(Context * context, MatisseParameters * mosaicParameters)
{
    qDebug() << logPrefix() << "configure";
    return configure(context, mosaicParameters);
}

void ImageProvider::callStart()
{
    qDebug() << logPrefix() << "start";
    start();
}

void ImageProvider::callStop()
{
    qDebug() << logPrefix() << "stop";
    stop();
}


