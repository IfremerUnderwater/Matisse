﻿#include "NavInfo.h"

// NAN is not defined in math.h (as normally required by C99) by VSC++
#ifdef WIN32
      int isnan(double x) { return x != x; }
      int isinf(double x) { return !isnan(x) && isnan(x - x); }
#endif

using namespace MatisseCommon;

NavInfo::NavInfo(quint32 dive, QDateTime time, qreal longitude, qreal latitude, qreal depth, qreal altitude, qreal yaw, qreal roll, qreal pitch, qreal vx, qreal vy, qreal vz)
{
    setInfo(dive, time, longitude, latitude, depth, altitude, yaw, roll, pitch, vx, vy, vz);
}

NavInfo::NavInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz)
{
    setInfo(dive, time, longitude, latitude, depth, altitude, yaw, roll, pitch, vx, vy, vz);
}

void NavInfo::setInfo(quint32 dive, QDateTime time, qreal longitude, qreal latitude, qreal depth, qreal altitude, qreal yaw, qreal roll, qreal pitch, qreal vx, qreal vy, qreal vz)
{
    _diveNumber = dive;
    _timeInfo = time;
    _longitude = longitude;
    _latitude = latitude;
    _depth = depth;
    _altitude = altitude;
    _yaw = yaw;
    _roll = roll;
    _pitch = pitch;
    _vx = vx;
    _vy = vy;
    _vz = vz;
}

void NavInfo::setInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz)
{
    setDiveNumber(dive);
    setTimeInfo(time);
    /*setLongitude(longitude);
    setLatitude(latitude);*/
    setLatLon(latitude, longitude);
    setDepth(depth);
    setAltitude(altitude);
    setYaw(yaw);
    setRoll(roll);
    setPitch(pitch);
    setVx(vx);
    setVy(vy);
    setVz(vz);
}

void NavInfo::setInfo(QStringList args)
{
    // int delta = 13 - args.length();
    // if (delta > 0) {
    //     args.append(QStringList());
    // }
    args.reserve(13);

    setDiveNumber(args[0]);
    setTimeInfo(args[1]);
    /*setLongitude(args[2]);
    setLatitude(args[3]);*/
    setLatLon(args[3],args[2]);
    setDepth(args[4]);
    setAltitude(args[5]);
    setYaw(args[6]);
    setRoll(args[7]);
    setPitch(args[8]);
    setVx(args[9]);
    setVy(args[10]);
    setVz(args[11]);
}

qreal NavInfo::vz() const
{
    return _vz;
}

void NavInfo::setVz(const qreal &vz)
{
    _vz = vz;
}

void NavInfo::setVz(const QString &arg)
{
    bool ok;
    _vz = arg.toDouble(&ok);
    if (!ok) {
        _vz = InvalidValue;
    }

}

qreal NavInfo::vy() const
{
    return _vy;
}

void NavInfo::setVy(const qreal &vy)
{
    _vy = vy;
}

void NavInfo::setVy(const QString &arg)
{
    bool ok;
    _vy = arg.toDouble(&ok);
    if (!ok) {
        _vy = InvalidValue;
    }
}

qreal NavInfo::vx() const
{
    return _vx;
}

void NavInfo::setVx(const qreal &vx)
{
    _vx = vx;
}

void NavInfo::setVx(const QString &arg)
{
    bool ok;
    _vx = arg.toDouble(&ok);
    if (!ok) {
        _vx = InvalidValue;
    }
}

qreal NavInfo::pitch() const
{
    return _pitch;
}

void NavInfo::setPitch(const qreal &pitch)
{
    _pitch = pitch;
}

void NavInfo::setPitch(const QString &arg)
{
    bool ok;
    _pitch = arg.toDouble(&ok);
    if (!ok) {
        _pitch = InvalidValue;
    } else {
        if ((_pitch < -180) || (_pitch > 180)) {
            _pitch = InvalidValue;
        }
    }
}

qreal NavInfo::roll() const
{
    return _roll;
}

void NavInfo::setRoll(const qreal &roll)
{
    _roll = roll;
}

void NavInfo::setRoll(const QString &arg)
{
    bool ok;
    _roll = arg.toDouble(&ok);
    if (!ok) {
        _roll = InvalidValue;
    } else {
        if ((_roll < -180) || (_roll > 180)) {
            _roll = InvalidValue;
        }
    }
}

qreal NavInfo::yaw() const
{
    return _yaw;
}

void NavInfo::setYaw(const qreal &yaw)
{
    _yaw = yaw;
}

void NavInfo::setYaw(const QString &arg)
{
    bool ok;
    _yaw = arg.toDouble(&ok);
    if (!ok) {
        _yaw = InvalidValue;
    } else {
        if ((_yaw < 0) || (_yaw > 360)) {
            _yaw = InvalidValue;
        }
    }
}

qreal NavInfo::altitude() const
{
    return _altitude;
}

void NavInfo::setAltitude(const qreal &altitude)
{
    _altitude = altitude;
}

void NavInfo::setAltitude(const QString &arg)
{
    bool ok;
    _altitude = arg.toDouble(&ok);
    if (!ok) {
        _altitude = InvalidValue;
    } else {
        if ((_altitude < 0) || (_altitude > 12000)) {
            _altitude = InvalidValue;
        }
    }
}

qreal NavInfo::depth() const
{
    return _depth;
}

void NavInfo::setDepth(const qreal &depth)
{
    _depth = depth;
}

void NavInfo::setDepth(const QString &arg)
{
    bool ok;
    _depth = arg.toDouble(&ok);
    if (!ok) {
        _depth = InvalidValue;
    } else {
        if ((_depth < 0) || (_depth > 12000)) {
            _depth = InvalidValue;
        }
    }
}

qreal NavInfo::latitude() const
{
    return _latitude;
}

void NavInfo::setLatLon(const qreal &latitude,const qreal &longitude)
{
    _latitude = latitude;
    _longitude = longitude;
}

void NavInfo::setLatLon(const QString &arg_lat, const QString &arg_lon)
{
    bool ok;

    _latitude = arg_lat.toDouble(&ok);
    if (!ok) {
        _latitude = InvalidValue;
    } else {
        if ((_latitude < -90) || (_latitude > 90)) {
            _latitude = InvalidValue;
        }
    }

    _longitude = arg_lon.toDouble(&ok);
    if (!ok) {
        _longitude = InvalidValue;
    } else {
        if ((_longitude < -180) || (_longitude > 180)) {
            _longitude = InvalidValue;
        }
    }
}

qreal NavInfo::longitude() const
{
    return _longitude;
}

QDateTime NavInfo::timeInfo() const
{
    return _timeInfo;
}

void NavInfo::setTimeInfo(const QDateTime &timeInfo)
{
    _timeInfo = timeInfo;
}

void NavInfo::setTimeInfo(const QString &arg)
{
    _timeInfo = QDateTime::fromString(arg.trimmed(), "dd/MM/yyyy HH:mm:ss.zzz");
}

quint32 NavInfo::diveNumber() const
{
    return _diveNumber;
}

void NavInfo::setDiveNumber(const quint32 &diveNumber)
{
    _diveNumber = diveNumber;
}

void NavInfo::setDiveNumber(const QString &arg)
{
    bool ok;
    _diveNumber = arg.toUInt(&ok);
    if (!ok) {
        _diveNumber = 0;
    }
}

bool NavInfo::isValid(QString flags)
{
    return ((diveNumber() >= 0) || (flags[0] == '0'))
            && (timeInfo().isValid() || (flags[1] == '0'))
            && (!isnan(longitude()) || (flags[2] == '0'))
            && (!isnan(latitude()) || (flags[3] == '0'))
            && (!isnan(depth()) || (flags[4] == '0'))
            && (!isnan(altitude()) || (flags[5] == '0'))
            && (!isnan(yaw()) || (flags[6] == '0'))
            && (!isnan(roll()) || (flags[7] == '0'))
            && (!isnan(pitch()) || (flags[8] == '0'));
            /* Mosaic can be done without vx, vy, vz
            && (!isnan(vx()) || (flags[9] == '0'))
            && (!isnan(vy()) || (flags[10] == '0'))
            && (!isnan(vz()) || (flags[11] == '0'));*/
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
qreal NavInfo::utmX() const
{
    return _utmX;
}

void NavInfo::setUtmX(const qreal &utmX)
{
    _utmX = utmX;
}
qreal NavInfo::utmY() const
{
    return _utmY;
}

void NavInfo::setUtmY(const qreal &utmY)
{
    _utmY = utmY;
}
QString NavInfo::utmZone() const
{
    return _utmZone;
}

void NavInfo::setUtmZone(const QString &utmZone)
{
    _utmZone = utmZone;
}


