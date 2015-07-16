#include "GeoTransform.h"
#include <proj_api.h>
#include <stdio.h>
#include <iostream>
#include <QDebug>
#include <cmath>

using namespace MatisseCommon;

GeoTransform::GeoTransform()
{
}

bool GeoTransform::LatLongToUTM(qreal lat_p, qreal lon_p, qreal & x_p, qreal & y_p, QString & utmZone_p, bool forceZone)
{

    projPJ pj_latlong, pj_utm;

    QString utmProjParam, utmHemisphereOption;

    if (!forceZone){
        int Zone;
        Zone = floor( ( lon_p / 6 ) + 31);

        if (lat_p>0){
            utmZone_p = QString("%1").arg(Zone)+ QString(" N");
        }else{
            utmZone_p = QString("%1").arg(Zone)+ QString(" S");
        }
    }

    // Construct utm proj param options
    QStringList utmParams = utmZone_p.split(" ");

    if ( utmParams.at(1) == "S" ){
        utmHemisphereOption = QString(" +south");
    }else{
        utmHemisphereOption = QString("");
    }
    utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0) + QString(" +ellps=WGS84");

    // Create projections
    if (!(pj_latlong = pj_init_plus("+proj=longlat +datum=WGS84")) ){
        std::cerr<<"pj_init_plus error: longlat\n";
        exit(1);
    }

    if (!(pj_utm = pj_init_plus(utmProjParam.toLocal8Bit().data())) ){
        std::cerr<<"pj_init_plus error: utm\n";
        exit(1);
    }

    qDebug() << "utmProjParam = " << utmProjParam;


    // Initialize x_p & y_p
    x_p = DEG_TO_RAD*lon_p;
    y_p = DEG_TO_RAD*lat_p;

    // transform points
    int p = pj_transform(pj_latlong, pj_utm, 1, 1, &x_p, &y_p, NULL );
    qDebug()<<"Error code: " << p <<" Error message: "<<  pj_strerrno(p) << "\n";

    return (p>=0);

}

bool GeoTransform::UTMToLatLong(qreal x_p, qreal y_p, QString utmZone_p, qreal &lat_p, qreal &lon_p)
{

    projPJ pj_latlong, pj_utm;

    QString utmProjParam, utmHemisphereOption;

    // Construct utm proj param options
    QStringList utmParams = utmZone_p.split(" ");

    if ( utmParams.at(1) == "S" ){
        utmHemisphereOption = QString(" +south");
    }else{
        utmHemisphereOption = QString("");
    }
    utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0) + QString(" +ellps=WGS84");

    // Create projections
    if (!(pj_latlong = pj_init_plus("+proj=longlat +datum=WGS84")) ){
        std::cerr<<"pj_init_plus error: longlat\n";
        exit(1);
    }
    if (!(pj_utm = pj_init_plus(utmProjParam.toLocal8Bit().data())) ){
        std::cerr<<"pj_init_plus error: utm\n";
        exit(1);
    }

    // Initialize lat_p & lon_p
    lon_p = x_p;
    lat_p = y_p;

    // transform points
    int p = pj_transform(pj_utm, pj_latlong, 1, 1, &lon_p, &lat_p, NULL );
    lat_p *= RAD_TO_DEG;
    lon_p *= RAD_TO_DEG;
    qDebug()<<"Error code: " << p <<" Error message: "<<  pj_strerrno(p) << "\n";
    printf("%f°\t%f°\n", lat_p, lon_p);

    return (p>=0);

}

cv::Mat GeoTransform::RotX(qreal a)
{

    cv::Mat R = (cv::Mat_<qreal>(3,3) <<
                 1,       0,       0,
                 0,  cos(a), -sin(a),
                 0,  sin(a),  cos(a));

    return R;

}

cv::Mat GeoTransform::RotY(qreal a)
{
    cv::Mat R = (cv::Mat_<qreal>(3,3) <<
                 cos(a),       0,  sin(a),
                 0,       1,       0,
                 -sin(a),       0,  cos(a));

    return R;
}

cv::Mat GeoTransform::RotZ(qreal a)
{
    cv::Mat R = (cv::Mat_<qreal>(3,3) <<
                 cos(a), -sin(a),       0,
                 sin(a),  cos(a),       0,
                 0,       0,       1 );

    return R;
}
