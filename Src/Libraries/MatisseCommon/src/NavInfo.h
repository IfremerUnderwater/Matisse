#ifndef NAVINFO_H
#define NAVINFO_H



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


namespace MatisseCommon {

#define InvalidValue NAN

///
/// \brief Classe de stockage des informations de navigation
///
class NavInfo
{
public:
    NavInfo(quint32 dive = 0, QDateTime time = QDateTime(), double longitude = InvalidValue, double latitude = InvalidValue, double depth = InvalidValue, double altitude = InvalidValue, double yaw = InvalidValue, double roll = InvalidValue, double pitch = InvalidValue, double vx = InvalidValue, double vy = InvalidValue, double vz = InvalidValue);
    NavInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);

    void setInfo(quint32 dive = 0, QDateTime time = QDateTime(), double longitude = InvalidValue, double latitude = InvalidValue, double depth = InvalidValue, double altitude = InvalidValue, double yaw = InvalidValue, double roll = InvalidValue, double pitch = InvalidValue, double vx = InvalidValue, double vy = InvalidValue, double vz = InvalidValue, double pan=0.0, double tilt=-M_PI_2);
    void setInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);
    void setInfo(QStringList args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &diveNumber);
    void setDiveNumber(const QString &arg);

    QDateTime timeInfo() const;
    void setTimeInfo(const QDateTime &timeInfo);
    void setTimeInfo(const QString &arg);

    double longitude() const;
    double latitude() const;
    void setLatLon(const double &latitude,const double &longitude);
    void setLatLon(const QString &arg_lat, const QString &arg_lon);

    double depth() const;
    void setDepth(const double &depth);
    void setDepth(const QString &arg);

    double altitude() const;
    void setAltitude(const double &altitude);
    void setAltitude(const QString &arg);

    double yaw() const;
    void setYaw(const double &yaw);
    void setYaw(const QString &arg);

    double roll() const;
    void setRoll(const double &roll);
    void setRoll(const QString &arg);

    double pitch() const;
    void setPitch(const double &pitch);
    void setPitch(const QString &arg);

    double vx() const;
    void setVx(const double &vx);
    void setVx(const QString &arg);

    double vy() const;
    void setVy(const double &vy);
    void setVy(const QString &arg);

    double vz() const;
    void setVz(const double &vz);
    void setVz(const QString &arg);

    bool isValid(QString flags = "11111111");

    QString dump();

    double utmX() const;
    void setUtmX(const double &utmX);

    double utmY() const;
    void setUtmY(const double &utmY);

    QString utmZone() const;
    void setUtmZone(const QString &utmZone);

    double pan() const;
    void setPan(const double &pan);

    double tilt() const;
    void setTilt(const double &tilt);

private:
    quint32 _diveNumber;
    QDateTime _timeInfo;
    double _utmX;
    double _utmY;
    QString _utmZone;
    double _longitude;
    double _latitude;
    double _depth;
    double _altitude;
    double _yaw;
    double _roll;
    double _pitch;
    double _vx;
    double _vy;
    double _vz;

    double _pan;
    double _tilt;
};
}
#endif // NAVINFO_H
