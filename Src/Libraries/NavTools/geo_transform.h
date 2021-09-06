#ifndef NAV_TOOLS_GEO_TRANSFORM_H_
#define NAV_TOOLS_GEO_TRANSFORM_H_



#include <QtGlobal>
#include <QDateTime>
#include <QByteArray>
#include <QStringList>
#include <opencv2/opencv.hpp>

namespace nav_tools {

///
/// \brief The GeoTransform class is based on the Proj4, OpenCV and Qt libraries and provide geographic & geometric operations
///
class GeoTransform
{
public:
    GeoTransform();

    ///
    /// \brief latLongToUTM Convert latitude and longitude to UTM
    /// \param _lat_p : latitude
    /// \param _lon_p : longitude
    /// \param _x_p : utm easting
    /// \param _y_p : utm northing
    /// \param _utm_zone_p : zone utm (output if forceZone==false, input and taken as the utm zone otherwise) format ex : "30 N"
    /// \param _force_zone
    /// \return true on success, false on fail
    ///
    bool latLongToUTM(double _lat_p, double _lon_p, double & _x_p, double & _y_p, QString & _utm_zone_p, bool _force_zone = false);

    ///
    /// \brief UTMToLatLong
    /// \param _x_p : utm easting
    /// \param _y_p : utm northing
    /// \param _utm_zone_p : zone utm, format ex : "30 N"
    /// \param _lat_p : latitude
    /// \param _lon_p : longitude
    /// \return true on success, false on fail
    ///
    bool UTMToLatLong(double _x_p, double _y_p, QString _utm_zone_p, double &_lat_p, double &_lon_p);

    ///
    /// \brief RotX construct a rotation matrix of angle "a" around x axis
    /// \param _a : rotation angle in rad
    /// \return the rotation matrix
    ///
    cv::Mat rotX( double _a );

    ///
    /// \brief RotY construct a rotation matrix of angle "a" around y axis
    /// \param _a : rotation angle in rad
    /// \return the rotation matrix
    ///
    cv::Mat rotY( double _a );

    ///
    /// \brief RotZ construct a rotation matrix of angle "a" around z axis
    /// \param a : rotation angle in rad
    /// \return the rotation matrix
    ///
    cv::Mat rotZ( double _a );
};

}

#endif // NAV_TOOLS_GEO_TRANSFORM_H_
