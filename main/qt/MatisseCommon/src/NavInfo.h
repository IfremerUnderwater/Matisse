#ifndef NAVINFO_H
#define NAVINFO_H

#include <QDateTime>
#include <QStringList>

namespace MatisseCommon {

#define InvalidValue 999999

///
/// \brief Classe de stockage des informations de navigation
///
class NavInfo
{
public:
    NavInfo(quint32 dive = 0, QDateTime time = QDateTime(), qreal longitude = InvalidValue, qreal latitude = InvalidValue, qreal depth = InvalidValue, qreal altitude = InvalidValue, qreal yaw = InvalidValue, qreal roll = InvalidValue, qreal pitch = InvalidValue, qreal vx = InvalidValue, qreal vy = InvalidValue, qreal vz = InvalidValue);
    NavInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);

    void setInfo(quint32 dive = 0, QDateTime time = QDateTime(), qreal longitude = InvalidValue, qreal latitude = InvalidValue, qreal depth = InvalidValue, qreal altitude = InvalidValue, qreal yaw = InvalidValue, qreal roll = InvalidValue, qreal pitch = InvalidValue, qreal vx = InvalidValue, qreal vy = InvalidValue, qreal vz = InvalidValue);
    void setInfo(QString dive, QString time, QString longitude, QString latitude, QString depth, QString altitude, QString yaw, QString roll, QString pitch, QString vx, QString vy, QString vz);
    void setInfo(QStringList args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &diveNumber);
    void setDiveNumber(const QString &arg);

    QDateTime timeInfo() const;
    void setTimeInfo(const QDateTime &timeInfo);
    void setTimeInfo(const QString &arg);

    qreal longitude() const;
    void setLongitude(const qreal &longitude);
    void setLongitude(const QString &arg);

    qreal latitude() const;
    void setLatitude(const qreal &latitude);
    void setLatitude(const QString &arg);

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

    bool isValid(QString flags = "111111111111");

    QString dump();


private:
    quint32 _diveNumber;
    QDateTime _timeInfo;
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
};
}
#endif // NAVINFO_H
