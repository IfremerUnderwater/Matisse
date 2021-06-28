#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H

#include "geo_transform.h"
#include <proj_api.h>
#include <stdio.h>
#include <iostream>
#include <QDebug>
#include <cmath>

namespace nav_tools {

GeoTransform::GeoTransform()
{
}

bool GeoTransform::latLongToUTM(double _lat_p, double _lon_p, double & _x_p, double & _y_p, QString & _utm_zone_p, bool _force_zone)
{

    projPJ pj_latlong, pj_utm;

    QString utm_proj_param, utm_hemisphere_option;

    if (!_force_zone){
        int Zone;
        Zone = floor( ( _lon_p / 6 ) + 31);

        if (_lat_p>0){
            _utm_zone_p = QString("%1").arg(Zone)+ QString(" N");
        }else{
            _utm_zone_p = QString("%1").arg(Zone)+ QString(" S");
        }
    }

    // Construct utm proj param options
    QStringList utm_params = _utm_zone_p.split(" ");

    if ( utm_params.at(1) == "S" ){
        utm_hemisphere_option = QString(" +south");
    }else{
        utm_hemisphere_option = QString("");
    }
    utm_proj_param = QString("+proj=utm +zone=") + utm_params.at(0) + QString(" +ellps=WGS84");

    // Create projections
    if (!(pj_latlong = pj_init_plus("+proj=longlat +datum=WGS84")) ){
        std::cerr<<"pj_init_plus error: longlat\n";
        exit(1);
    }

    if (!(pj_utm = pj_init_plus(utm_proj_param.toLocal8Bit().data())) ){
        std::cerr<<"pj_init_plus error: utm\n";
        exit(1);
    }

    qDebug() << "utmProjParam = " << utm_proj_param;


    // Initialize x_p & y_p
    _x_p = DEG_TO_RAD*_lon_p;
    _y_p = DEG_TO_RAD*_lat_p;

    // transform points
    int p = pj_transform(pj_latlong, pj_utm, 1, 1, &_x_p, &_y_p, NULL );
    qDebug()<<"Error code: " << p <<" Error message: "<<  pj_strerrno(p) << "\n";

    return (p>=0);

}

bool GeoTransform::UTMToLatLong(double _x_p, double _y_p, QString _utm_zone_p, double &_lat_p, double &_lon_p)
{

    projPJ pj_latlong, pj_utm;

    QString utm_proj_param, utm_hemisphere_option;

    // Construct utm proj param options
    QStringList utm_params = _utm_zone_p.split(" ");

    if ( utm_params.at(1) == "S" ){
        utm_hemisphere_option = QString(" +south");
    }else{
        utm_hemisphere_option = QString("");
    }
    utm_proj_param = QString("+proj=utm +zone=") + utm_params.at(0) + QString(" +ellps=WGS84");

    // Create projections
    if (!(pj_latlong = pj_init_plus("+proj=longlat +datum=WGS84")) ){
        std::cerr<<"pj_init_plus error: longlat\n";
        exit(1);
    }
    if (!(pj_utm = pj_init_plus(utm_proj_param.toLocal8Bit().data())) ){
        std::cerr<<"pj_init_plus error: utm\n";
        exit(1);
    }

    // Initialize lat_p & lon_p
    _lon_p = _x_p;
    _lat_p = _y_p;

    // transform points
    int p = pj_transform(pj_utm, pj_latlong, 1, 1, &_lon_p, &_lat_p, NULL );
    _lat_p *= RAD_TO_DEG;
    _lon_p *= RAD_TO_DEG;
    qDebug()<<"Error code: " << p <<" Error message: "<<  pj_strerrno(p) << "\n";
    printf("%f°\t%f°\n", _lat_p, _lon_p);

    return (p>=0);

}

cv::Mat GeoTransform::rotX(double _a)
{

    cv::Mat R = (cv::Mat_<double>(3,3) <<
                 1,       0,       0,
                 0,  cos(_a), -sin(_a),
                 0,  sin(_a),  cos(_a));

    return R;

}

cv::Mat GeoTransform::rotY(double _a)
{
    cv::Mat R = (cv::Mat_<double>(3,3) <<
                 cos(_a),       0,  sin(_a),
                 0,       1,       0,
                 -sin(_a),       0,  cos(_a));

    return R;
}

cv::Mat GeoTransform::rotZ(double _a)
{
    cv::Mat R = (cv::Mat_<double>(3,3) <<
                 cos(_a), -sin(_a),       0,
                 sin(_a),  cos(_a),       0,
                 0,       0,       1 );

    return R;
}

} // namespace nav_tools
