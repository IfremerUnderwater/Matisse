#ifndef NAV_TOOLS_CARTO_IMAGE_H_
#define NAV_TOOLS_CARTO_IMAGE_H_

#include "image.h"

#include <QRect>
#include <QPoint>

using namespace matisse_image;

namespace nav_tools {

class CartoImage : public Image
{
public:
    CartoImage();
    bool loadFile(QString _filename);

    typedef double GeoTransform[6];
    GeoTransform & getGeoTransform() {
        return m_adf_geo_transform;
    }

    // returns envelope (with conversion from GeoTIFF)
    QRectF getEnvelope();

    // conversions
    //Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)
    qreal xGeo(qreal x_pixel, qreal _y_line)
    {
        return m_adf_geo_transform[0] + x_pixel*m_adf_geo_transform[1] + _y_line*m_adf_geo_transform[2];
    }

    //Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)
    qreal yGeo(qreal _x_pixel, qreal _y_line)
    {
        // !!!! row order (positive up in geographic coords, positive down in device coordinates)
        return -(m_adf_geo_transform[3] + _x_pixel*m_adf_geo_transform[4] + _y_line*m_adf_geo_transform[5]);
    }

    // width of image (and not containing envelope)
    qreal widthGeo();

    // height of image (and not containing envelope)
    qreal heightGeo();

    // in rd
    qreal getRotationAngle();

    QString fileName() const { return m_file_name; }
private:
    GeoTransform m_adf_geo_transform;
    QString m_file_name;
};

} // namespace nav_tools

#endif // NAV_TOOLS_CARTO_IMAGE_H_
