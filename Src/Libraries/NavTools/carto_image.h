#ifndef CARTOIMAGE_H
#define CARTOIMAGE_H

#include "image.h"

#include <QRect>
#include <QPoint>

using namespace matisse_image;

class CartoImage : public Image
{
public:
    CartoImage();
    bool loadFile(QString filename_p);

    typedef double GeoTransform[6];
    GeoTransform & getGeoTransform() {
        return _adfGeoTransform;
    }

    // returns envelope (with conversion from GeoTIFF)
    QRectF getEnvelope();

    // conversions
    //Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)
    qreal xGeo(qreal XPixel, qreal YLine)
    {
        return _adfGeoTransform[0] + XPixel*_adfGeoTransform[1] + YLine*_adfGeoTransform[2];
    }

    //Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)
    qreal yGeo(qreal XPixel, qreal YLine)
    {
        // !!!! row order (positive up in geographic coords, positive down in device coordinates)
        return -(_adfGeoTransform[3] + XPixel*_adfGeoTransform[4] + YLine*_adfGeoTransform[5]);
    }

    // width of image (and not containing envelope)
    qreal widthGeo();

    // height of image (and not containing envelope)
    qreal heightGeo();

    // in rd
    qreal getRotationAngle();

    QString fileName() const { return _fileName; }
private:
    GeoTransform _adfGeoTransform;
    QString _fileName;
};

#endif // CARTOIMAGE_H
