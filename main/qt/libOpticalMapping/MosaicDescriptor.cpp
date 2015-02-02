#include "MosaicDescriptor.h"


MosaicDescriptor::MosaicDescriptor():_mosaicOrigin(0,0,0),
    _pixelSize(0,0),_mosaicSize(0,0), _utmHemisphere("UNDEF"),_utmZone(-1)
{

    Mat* _mosaic_ullr = new Mat(1,4,CV_64F);

}

MosaicDescriptor::~MosaicDescriptor()
{
    delete _mosaic_ullr;
}

Point3d MosaicDescriptor::mosaicOrigin() const
{
    return _mosaicOrigin;
}

void MosaicDescriptor::setMosaicOrigin(const Point3d &mosaicOrigin)
{
    _mosaicOrigin = mosaicOrigin;
}
Point2d MosaicDescriptor::pixelSize() const
{
    return _pixelSize;
}

void MosaicDescriptor::setPixelSize(const Point2d &pixelSize)
{
    _pixelSize = pixelSize;
}
Point2d MosaicDescriptor::mosaicSize() const
{
    return _mosaicSize;
}

void MosaicDescriptor::setMosaicSize(const Point2d &mosaicSize)
{
    _mosaicSize = mosaicSize;
}
QString MosaicDescriptor::utmHemisphere() const
{
    return _utmHemisphere;
}

void MosaicDescriptor::setUtmHemisphere(const QString &utmHemisphere)
{
    _utmHemisphere = utmHemisphere;
}
int MosaicDescriptor::utmZone() const
{
    return _utmZone;
}

void MosaicDescriptor::setUtmZone(int utmZone)
{
    _utmZone = utmZone;
}
QVector<Mnode> MosaicDescriptor::imagesNodes() const
{
    return _imagesNodes;
}

void MosaicDescriptor::setImagesNodes(const QVector<Mnode> &imagesNodes)
{
    _imagesNodes = imagesNodes;
}
Mat *MosaicDescriptor::mosaic_ullr() const
{
    return _mosaic_ullr;
}

void MosaicDescriptor::setMosaic_ullr(Mat *mosaic_ullr)
{
    _mosaic_ullr = mosaic_ullr;
}






