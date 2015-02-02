#ifndef GEOTRANSFORM_H
#define GEOTRANSFORM_H

#include <QtGlobal>
#include <QDateTime>
#include <QByteArray>
#include <QStringList>

///
/// \brief The GeoTransform class is based on the Proj4 library and provide geographic coordinates operations
///
class GeoTransform
{
public:
    GeoTransform();

    ///
    /// \brief LatLongToUTM Convert latitude and longitude to UTM
    /// \param lat_p : latitude
    /// \param lon_p : longitude
    /// \param x_p : utm easting
    /// \param y_p : utm northing
    /// \param utmZone_p : zone utm (output if forceZone==false, input and taken as the utm zone otherwise) format ex : "30 N"
    /// \param forceZone
    /// \return true on success, false on fail
    ///
    bool LatLongToUTM(qreal lat_p, qreal lon_p, qreal & x_p, qreal & y_p, QString & utmZone_p, bool forceZone = false);

    ///
    /// \brief UTMToLatLong
    /// \param x_p : utm easting
    /// \param y_p : utm northing
    /// \param utmZone_p : zone utm, format ex : "30 N"
    /// \param lat_p : latitude
    /// \param lon_p : longitude
    /// \return true on success, false on fail
    ///
    bool UTMToLatLong(qreal x_p, qreal y_p, QString utmZone_p, qreal &lat_p, qreal &lon_p);
};

#endif // GEOTRANSFORM_H
