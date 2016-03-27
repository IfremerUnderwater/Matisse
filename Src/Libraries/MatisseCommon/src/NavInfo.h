#ifndef NAVINFO_H
#define NAVINFO_H

#include <QDateTime>
#include <QStringList>
#include <cmath>

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
    NavInfo(quint32 dive = 0, QDateTime time = QDateTime(), qreal longitude = InvalidValue, qreal latitude = InvalidValue, qreal depth = InvalidValue, qreal altitude = InvalidValue, qreal yaw = InvalidValue, qreal roll = InvalidValue, qreal pitch = InvalidValue, qreal vx = InvalidValue, qreal vy = InvalidValue, qreal vz = InvalidValue);
    NavInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);

    void setInfo(quint32 dive = 0, QDateTime time = QDateTime(), qreal longitude = InvalidValue, qreal latitude = InvalidValue, qreal depth = InvalidValue, qreal altitude = InvalidValue, qreal yaw = InvalidValue, qreal roll = InvalidValue, qreal pitch = InvalidValue, qreal vx = InvalidValue, qreal vy = InvalidValue, qreal vz = InvalidValue, qreal pan=0.0, qreal tilt=0.0);
    void setInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);
    void setInfo(QStringList args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &diveNumber);
    void setDiveNumber(const QString &arg);

    QDateTime timeInfo() const;
    void setTimeInfo(const QDateTime &timeInfo);
    void setTimeInfo(const QString &arg);

    qreal longitude() const;
    qreal latitude() const;
    void setLatLon(const qreal &latitude,const qreal &longitude);
    void setLatLon(const QString &arg_lat, const QString &arg_lon);

    qreal depth() const;
    void setDepth(const qreal &depth);
    void setDepth(const QString &arg);

    qreal altitude() const;
    void setAltitude(const qreal &altitude);
    void setAltitude(const QString &arg);

    qreal yaw() const;
    void setYaw(const qreal &yaw);
    void setYaw(const QString &arg);

    qreal roll() const;
    void setRoll(const qreal &roll);
    void setRoll(const QString &arg);

    qreal pitch() const;
    void setPitch(const qreal &pitch);
    void setPitch(const QString &arg);

    qreal vx() const;
    void setVx(const qreal &vx);
    void setVx(const QString &arg);

    qreal vy() const;
    void setVy(const qreal &vy);
    void setVy(const QString &arg);

    qreal vz() const;
    void setVz(const qreal &vz);
    void setVz(const QString &arg);

    bool isValid(QString flags = "111111111");

    QString dump();

    qreal utmX() const;
    void setUtmX(const qreal &utmX);

    qreal utmY() const;
    void setUtmY(const qreal &utmY);

    QString utmZone() const;
    void setUtmZone(const QString &utmZone);

    qreal pan() const;
    void setPan(const qreal &pan);

    qreal tilt() const;
    void setTilt(const qreal &tilt);

private:
    quint32 _diveNumber;
    QDateTime _timeInfo;
    qreal _utmX;
    qreal _utmY;
    QString _utmZone;
    qreal _longitude;
    qreal _latitude;
    qreal _depth;
    qreal _altitude;
    qreal _yaw;
    qreal _roll;
    qreal _pitch;
    qreal _vx;
    qreal _vy;
    qreal _vz;

    qreal _pan;
    qreal _tilt;
};
}
#endif // NAVINFO_H
