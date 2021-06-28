#ifndef MATISSE_IMAGE_NAV_INFO_H_
#define MATISSE_IMAGE_NAV_INFO_H_



#include <QDateTime>
#include <QStringList>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <math.h>
#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923
#endif

// NAN is not defined in math.h (as normally required by C99) by VSC++
#ifdef WIN32
  #ifndef NAN
      static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};
      #define NAN (*(const float *) __nan)

  #endif
#endif


namespace matisse_image {

#define InvalidValue NAN

///
/// \brief Classe de stockage des informations de navigation
///
class NavInfo
{
public:
    NavInfo(quint32 _dive = 0, QDateTime _time = QDateTime(), double _longitude = InvalidValue, double _latitude = InvalidValue, double _depth = InvalidValue, double _altitude = InvalidValue, double _yaw = InvalidValue, double _roll = InvalidValue, double _pitch = InvalidValue, double _vx = InvalidValue, double _vy = InvalidValue, double _vz = InvalidValue);
    NavInfo(QString _dive, QString _time, QString _longitude, QString _latitude, QString _depth, QString _altitude, QString _yaw, QString _roll, QString _pitch, QString _vx, QString _vy, QString _vz);

    void setInfo(quint32 _dive = 0, QDateTime _time = QDateTime(), double _longitude = InvalidValue, double _latitude = InvalidValue, double _depth = InvalidValue, double _altitude = InvalidValue, double _yaw = InvalidValue, double _roll = InvalidValue, double _pitch = InvalidValue, double _vx = InvalidValue, double _vy = InvalidValue, double _vz = InvalidValue, double _pan=0.0, double _tilt=-M_PI_2);
    void setInfo(QString _dive, QString _time, QString _longitude, QString _latitude, QString _depth, QString _altitude, QString _yaw, QString _roll, QString _pitch, QString _vx, QString _vy, QString _vz);
    void setInfo(QStringList args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &_dive_number);
    void setDiveNumber(const QString &_arg);

    QDateTime timeInfo() const;
    void setTimeInfo(const QDateTime &_time_info);
    void setTimeInfo(const QString &_arg);

    double longitude() const;
    double latitude() const;
    void setLatLon(const double &_latitude,const double &_longitude);
    void setLatLon(const QString &_arg_lat, const QString &_arg_lon);

    double depth() const;
    void setDepth(const double &_depth);
    void setDepth(const QString &_arg);

    double altitude() const;
    void setAltitude(const double &_altitude);
    void setAltitude(const QString &_arg);

    double yaw() const;
    void setYaw(const double &_yaw);
    void setYaw(const QString &_arg);

    double roll() const;
    void setRoll(const double &_roll);
    void setRoll(const QString &_arg);

    double pitch() const;
    void setPitch(const double &_pitch);
    void setPitch(const QString &_arg);

    double vx() const;
    void setVx(const double &_vx);
    void setVx(const QString &_arg);

    double vy() const;
    void setVy(const double &_vy);
    void setVy(const QString &_arg);

    double vz() const;
    void setVz(const double &_vz);
    void setVz(const QString &_arg);

    bool isValid(QString flags = "11111111");

    QString dump();

    double utmX() const;
    void setUtmX(const double &_utm_x);

    double utmY() const;
    void setUtmY(const double &_utm_y);

    QString utmZone() const;
    void setUtmZone(const QString &_utm_zone);

    double pan() const;
    void setPan(const double &_pan);

    double tilt() const;
    void setTilt(const double &_tilt);

private:
    quint32 m_dive_number;
    QDateTime m_time_info;
    double m_utm_x;
    double m_utm_y;
    QString m_utm_zone;
    double m_longitude;
    double m_latitude;
    double m_depth;
    double m_altitude;
    double m_yaw;
    double m_roll;
    double m_pitch;
    double m_vx;
    double m_vy;
    double m_vz;

    double m_pan;
    double m_tilt;
};
}
#endif // MATISSE_IMAGE_NAV_INFO_H_
