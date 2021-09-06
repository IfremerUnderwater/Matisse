#include "nav_info.h"
#include <cmath>

using namespace std;
// NAN is not defined in math.h (as normally required by C99) by VSC++
//#ifdef WIN32
//      int isnan(double x) { return x != x; }
//      int isinf(double x) { return !isnan(x) && isnan(x - x); }
//#endif

namespace matisse_image {

NavInfo::NavInfo(quint32 _dive, QDateTime _time, double _longitude, double _latitude, double _depth, double _altitude, double _yaw, double _roll, double _pitch, double _vx, double _vy, double _vz)
{
    setInfo(_dive, _time, _longitude, _latitude, _depth, _altitude, _yaw, _roll, _pitch, _vx, _vy, _vz);
}

NavInfo::NavInfo(QString _dive, QString _time, QString _longitude, QString _latitude, QString _depth, QString _altitude, QString _yaw, QString _roll, QString _pitch, QString _vx, QString _vy, QString _vz)
{
    setInfo(_dive, _time, _longitude, _latitude, _depth, _altitude, _yaw, _roll, _pitch, _vx, _vy, _vz);
}

void NavInfo::setInfo(quint32 _dive, QDateTime _time, double _longitude, double _latitude, double _depth, double _altitude, double _yaw, double _roll, double _pitch, double _vx, double _vy, double _vz, double _pan, double _tilt)
{
    m_dive_number = _dive;
    m_time_info = _time;
    m_longitude = _longitude;
    m_latitude = _latitude;
    m_depth = _depth;
    m_altitude = _altitude;
    m_yaw = _yaw;
    m_roll = _roll;
    m_pitch = _pitch;
    m_vx = _vx;
    m_vy = _vy;
    m_vz = _vz;
    m_pan = _pan;
    _tilt = _tilt;
}

void NavInfo::setInfo(QString _dive, QString _time, QString _longitude, QString _latitude, QString _depth, QString _altitude, QString _yaw, QString _roll, QString _pitch, QString _vx, QString _vy, QString _vz)
{
    setDiveNumber(_dive);
    setTimeInfo(_time);
    /*setLongitude(longitude);
    setLatitude(latitude);*/
    setLatLon(_latitude, _longitude);
    setDepth(_depth);
    setAltitude(_altitude);
    setYaw(_yaw);
    setRoll(_roll);
    setPitch(_pitch);
    setVx(_vx);
    setVy(_vy);
    setVz(_vz);
}

void NavInfo::setInfo(QStringList _args)
{
    // int delta = 13 - args.length();
    // if (delta > 0) {
    //     args.append(QStringList());
    // }
    _args.reserve(13);

    setDiveNumber(_args[0]);
    setTimeInfo(_args[1]);
    /*setLongitude(args[2]);
    setLatitude(args[3]);*/
    setLatLon(_args[3],_args[2]);
    setDepth(_args[4]);
    setAltitude(_args[5]);
    setYaw(_args[6]);
    setRoll(_args[7]);
    setPitch(_args[8]);
    setVx(_args[9]);
    setVy(_args[10]);
    setVz(_args[11]);
}

double NavInfo::vz() const
{
    return m_vz;
}

void NavInfo::setVz(const double &_vz)
{
    m_vz = _vz;
}

void NavInfo::setVz(const QString &_arg)
{
    bool ok;
    m_vz = _arg.toDouble(&ok);
    if (!ok) {
        m_vz = InvalidValue;
    }

}

double NavInfo::vy() const
{
    return m_vy;
}

void NavInfo::setVy(const double &_vy)
{
    m_vy = _vy;
}

void NavInfo::setVy(const QString &_arg)
{
    bool ok;
    m_vy = _arg.toDouble(&ok);
    if (!ok) {
        m_vy = InvalidValue;
    }
}

double NavInfo::vx() const
{
    return m_vx;
}

void NavInfo::setVx(const double &_vx)
{
    m_vx = _vx;
}

void NavInfo::setVx(const QString &_arg)
{
    bool ok;
    m_vx = _arg.toDouble(&ok);
    if (!ok) {
        m_vx = InvalidValue;
    }
}

double NavInfo::pitch() const
{
    return m_pitch;
}

void NavInfo::setPitch(const double &_pitch)
{
    m_pitch = _pitch;
}

void NavInfo::setPitch(const QString &_arg)
{
    bool ok;
    m_pitch = _arg.toDouble(&ok);
    if (!ok) {
        m_pitch = InvalidValue;
    } else {
        if ((m_pitch < -180) || (m_pitch > 180)) {
            m_pitch = InvalidValue;
        }
    }
}

double NavInfo::roll() const
{
    return m_roll;
}

void NavInfo::setRoll(const double &_roll)
{
    m_roll = _roll;
}

void NavInfo::setRoll(const QString &_arg)
{
    bool ok;
    m_roll = _arg.toDouble(&ok);
    if (!ok) {
        m_roll = InvalidValue;
    } else {
        if ((m_roll < -180) || (m_roll > 180)) {
            m_roll = InvalidValue;
        }
    }
}

double NavInfo::yaw() const
{
    return m_yaw;
}

void NavInfo::setYaw(const double &_yaw)
{
    m_yaw = _yaw;
}

void NavInfo::setYaw(const QString &_arg)
{
    bool ok;
    m_yaw = _arg.toDouble(&ok);
    if (!ok) {
        m_yaw = InvalidValue;
    } else {
        if ((m_yaw < -360) || (m_yaw > 360)) {
            m_yaw = InvalidValue;
        }
    }
}

double NavInfo::altitude() const
{
    return m_altitude;
}

void NavInfo::setAltitude(const double &_altitude)
{
    m_altitude = _altitude;
}

void NavInfo::setAltitude(const QString &_arg)
{
    bool ok;
    m_altitude = _arg.toDouble(&ok);
    if (!ok) {
        m_altitude = InvalidValue;
    } else {
        if ((m_altitude < 0) || (m_altitude > 12000)) {
            m_altitude = InvalidValue;
        }
    }
}

double NavInfo::depth() const
{
    return m_depth;
}

void NavInfo::setDepth(const double &_depth)
{
    m_depth = _depth;
}

void NavInfo::setDepth(const QString &_arg)
{
    bool ok;
    m_depth = _arg.toDouble(&ok);
    if (!ok) {
        m_depth = InvalidValue;
    } else {
        if ((m_depth < 0) || (m_depth > 12000)) {
            m_depth = InvalidValue;
        }
    }
}

double NavInfo::latitude() const
{
    return m_latitude;
}

void NavInfo::setLatLon(const double &_latitude, const double &_longitude)
{
    m_latitude = _latitude;
    m_longitude = _longitude;
}

void NavInfo::setLatLon(const QString &_arg_lat, const QString &_arg_lon)
{
    bool ok;

    m_latitude = _arg_lat.toDouble(&ok);
    if (!ok) {
        m_latitude = InvalidValue;
    } else {
        if ((m_latitude < -90) || (m_latitude > 90)) {
            m_latitude = InvalidValue;
        }
    }

    m_longitude = _arg_lon.toDouble(&ok);
    if (!ok) {
        m_longitude = InvalidValue;
    } else {
        if ((m_longitude < -180) || (m_longitude > 180)) {
            m_longitude = InvalidValue;
        }
    }
}

double NavInfo::longitude() const
{
    return m_longitude;
}

QDateTime NavInfo::timeInfo() const
{
    return m_time_info;
}

void NavInfo::setTimeInfo(const QDateTime &_time_info)
{
    m_time_info = _time_info;
}

void NavInfo::setTimeInfo(const QString &_arg)
{
    m_time_info = QDateTime::fromString(_arg.trimmed(), "dd/MM/yyyy HH:mm:ss.zzz");
}

quint32 NavInfo::diveNumber() const
{
    return m_dive_number;
}

void NavInfo::setDiveNumber(const quint32 &_dive_number)
{
    m_dive_number = _dive_number;
}

void NavInfo::setDiveNumber(const QString &_arg)
{
    bool ok;
    m_dive_number = _arg.toUInt(&ok);
    if (!ok) {
        m_dive_number = 0;
    }
}

bool NavInfo::isValid(QString flags)
{
#ifdef WIN32
    return ((timeInfo().isValid() || (flags[1] == '0'))
            && (!isnan(longitude()) || (flags[2] == '0'))
            && (!isnan(latitude()) || (flags[3] == '0'))
            && (!isnan(depth()) || (flags[4] == '0'))
            && (!isnan(altitude()) || (flags[5] == '0'))
            && (!isnan(yaw()) || (flags[6] == '0'))
            && (!isnan(roll()) || (flags[7] == '0'))
            && (!isnan(pitch()) || (flags[8] == '0')));
            /* Mosaic can be done without vx, vy, vz
            && (!isnan(vx()) || (flags[9] == '0'))
            && (!isnan(vy()) || (flags[10] == '0'))
            && (!isnan(vz()) || (flags[11] == '0'));*/
#else
    return ((timeInfo().isValid() || (flags[1] == '0'))
            && (!std::isnan(longitude()) || (flags[2] == '0'))
            && (!std::isnan(latitude()) || (flags[3] == '0'))
            && (!std::isnan(depth()) || (flags[4] == '0'))
            && (!std::isnan(altitude()) || (flags[5] == '0'))
            && (!std::isnan(yaw()) || (flags[6] == '0'))
            && (!std::isnan(roll()) || (flags[7] == '0'))
            && (!std::isnan(pitch()) || (flags[8] == '0')));
            /* Mosaic can be done without vx, vy, vz
            && (!isnan(vx()) || (flags[9] == '0'))
            && (!isnan(vy()) || (flags[10] == '0'))
            && (!isnan(vz()) || (flags[11] == '0'));*/
#endif
}

QString NavInfo::dump()
{
    QString ret("NavInfo:\n");
    ret += QString("\tdiveNumber = %1\n").arg(diveNumber());
    ret += QString("\tlatitude = %1\n").arg(latitude());
    ret += QString("\tlongitude = %1\n").arg(longitude());
    ret += QString("\tdepth = %1\n").arg(depth());
    ret += QString("\taltitude = %1\n").arg(altitude());
    ret += QString("\troll = %1\n").arg(roll());
    ret += QString("\tpitch = %1\n").arg(pitch());
    ret += QString("\tyaw = %1\n").arg(yaw());
    ret += QString("\tvx = %1\n").arg(vx());
    ret += QString("\tvy = %1\n").arg(vy());
    ret += QString("\tvz = %1\n").arg(vz());

    return ret;
}
double NavInfo::utmX() const
{
    return m_utm_x;
}

void NavInfo::setUtmX(const double &_utm_x)
{
    m_utm_x = _utm_x;
}
double NavInfo::utmY() const
{
    return m_utm_y;
}

void NavInfo::setUtmY(const double &_utm_y)
{
    m_utm_y = _utm_y;
}
QString NavInfo::utmZone() const
{
    return m_utm_zone;
}

void NavInfo::setUtmZone(const QString &_utm_zone)
{
    m_utm_zone = _utm_zone;
}
double NavInfo::pan() const
{
    return m_pan;
}

void NavInfo::setPan(const double &_pan)
{
    m_pan = _pan;
}
double NavInfo::tilt() const
{
    return m_tilt;
}

void NavInfo::setTilt(const double &_tilt)
{
    m_tilt = _tilt;
}

} // namespace matisse_image



