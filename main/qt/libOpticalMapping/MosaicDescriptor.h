#ifndef MOSAIC_H
#define MOSAIC_H

#include "libopticalmapping_global.h"
#include "NavImage.h"
#include <QVector>

using namespace MatisseCommon;

struct Mnode {

    NavImage* image;
    Mat* homo;

};

class LIBOPTICALMAPPINGSHARED_EXPORT MosaicDescriptor
{

public:
    MosaicDescriptor();
    ~MosaicDescriptor();


    Point3d mosaicOrigin() const;
    void setMosaicOrigin(const Point3d &mosaicOrigin);

    Point2d pixelSize() const;
    void setPixelSize(const Point2d &pixelSize);

    Point2d mosaicSize() const;
    void setMosaicSize(const Point2d &mosaicSize);

    QString utmHemisphere() const;
    void setUtmHemisphere(const QString &utmHemisphere);

    int utmZone() const;
    void setUtmZone(int utmZone);

    QVector<Mnode> imagesNodes() const;
    void setImagesNodes(const QVector<Mnode> &imagesNodes);

    Mat *mosaic_ullr() const;
    void setMosaic_ullr(Mat *mosaic_ullr);

protected:

    Point3d _mosaicOrigin;
    Point2d _pixelSize;
    Point2d _mosaicSize;
    Mat* _mosaic_ullr;
    QString _utmHemisphere;
    int _utmZone;

    QVector<Mnode> _imagesNodes;

};

#endif // MOSAIC_H
