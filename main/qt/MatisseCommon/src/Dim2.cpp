#include "Dim2.h"

using namespace MatisseCommon;

Dim2::Dim2(QString args):
    _validityArray(DIM2_FIELDS_NUMBER, '0')
{
    QStringList argsList = args.split(";");
    int nbArgs = argsList.length();
    bool ok;
    for(int noArg = 0; noArg < nbArgs; noArg++) {
        switch(noArg) {
            case 0: {
                // dive
                setDiveNumber(argsList.at(noArg).toULong(&ok));
            }
            break;
        case 1: {
            // date
            setDateTime(QDate::fromString(argsList.at(noArg).trimmed(), "dd/MM/yyyy"));
        }
        break;
        case 2: {
            // time
            QDateTime currentDate = dateTime();
            currentDate.setTime(QTime::fromString(argsList.at(noArg).trimmed(), "hh:mm:ss.zzz"));
            setDateTime(currentDate);
            ok = dateTime().isValid();
        }
        break;

        case 3: {
            // src
            setSource(argsList.at(noArg).trimmed());
            ok = !source().isEmpty();
         }
        break;

        case 4: {
            // format
            setFormat(argsList.at(noArg).trimmed());
            ok = !format().isEmpty();
         }
        break;

        case 5: {
            // name
            setFilename(argsList.at(noArg).trimmed());
            ok = !filename().isEmpty();
         }
        break;

        case 6: {
            // latitude
            setLatitude(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 7: {
            // longitude
            setLongitude(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 8: {
            // depth
            setDepth(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 9: {
            // altitude
            setAltitude(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 10: {
            // yaw
            setYaw(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 11: {
            // roll
            setRoll(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 12: {
            // pitch
            setPitch(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 13: {
            // vx
            setVx(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 14: {
            // vy
            setVy(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 15: {
            // vz
            setVz(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 16: {
            // vrx
            setVrx(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 17: {
            // vry
            setVry(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 18: {
            // vrz
            setVrz(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 19: {
            // main pan
            setMainPan(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 20: {
            // main tilt
            setMainTilt(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 21: {
            // main zoom
            setMainZoom(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 22: {
            // main focus
            setMainFocus(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 23: {
            // main iris
            setMainIris(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 24: {
            // vertical zoom
            setVerticalZoom(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 25: {
            // vertical focus
            setVerticalFocus(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 26: {
            // vertical iris
            setVerticalIris(argsList.at(noArg).toDouble(&ok));
         }
        break;

        case 27: {
            // event
            setEvent(argsList.at(noArg).trimmed());
            ok = !event().isEmpty();
         }
        break;

        case 28: {
            // tag
            setTag(argsList.at(noArg).trimmed());
            ok = !tag().isEmpty();
         }
        break;

        case 29: {
            // comment
            setComment(argsList.at(noArg).trimmed());
            ok = !comment().isEmpty();
         }
        break;
        default:
        break;
    }

        char * flag = "1";
        if (!ok) {
            flag = "0";
        }

        if (noArg == 0) {
                _validityArray.replace(0, 1, flag);
        } else if (noArg > 1) {
            _validityArray.replace(noArg-1, 1, flag);
        }

    }

}

QString Dim2::comment() const
{
    return _comment;
}

void Dim2::setComment(const QString &comment)
{
    _comment = comment;
}
qreal Dim2::mainZoom() const
{
    return _mainZoom;
}

void Dim2::setMainZoom(const qreal &mainZoom)
{
    _mainZoom = mainZoom;
}

quint32 Dim2::diveNumber() const
{
    return _diveNumber;
}

void Dim2::setDiveNumber(const quint32 &diveNumber)
{
    _diveNumber = diveNumber;
}
QDateTime Dim2::dateTime() const
{
    return _dateTime;
}

void Dim2::setDateTime(const QDateTime &dateTime)
{
    _dateTime = dateTime;
}

void Dim2::setDateTime(const QDate &date)
{
    _dateTime = QDateTime(date);
}

QString Dim2::source() const
{
    return _source;
}

void Dim2::setSource(const QString &source)
{
    _source = source;
}
QString Dim2::format() const
{
    return _format;
}

void Dim2::setFormat(const QString &format)
{
    _format = format;
}
QString Dim2::filename() const
{
    return _filename;
}

void Dim2::setFilename(const QString &filename)
{
    _filename = filename;
}
qreal Dim2::latitude() const
{
    return _latitude;
}

void Dim2::setLatitude(const qreal &latitude)
{
    _latitude = latitude;
}
qreal Dim2::longitude() const
{
    return _longitude;
}

void Dim2::setLongitude(const qreal &longitude)
{
    _longitude = longitude;
}
qreal Dim2::depth() const
{
    return _depth;
}

void Dim2::setDepth(const qreal &depth)
{
    _depth = depth;
}
qreal Dim2::altitude() const
{
    return _altitude;
}

void Dim2::setAltitude(const qreal &altitude)
{
    _altitude = altitude;
}
qreal Dim2::yaw() const
{
    return _yaw;
}

void Dim2::setYaw(const qreal &yaw)
{
    _yaw = yaw;
}
qreal Dim2::roll() const
{
    return _roll;
}

void Dim2::setRoll(const qreal &roll)
{
    _roll = roll;
}
qreal Dim2::pitch() const
{
    return _pitch;
}

void Dim2::setPitch(const qreal &pitch)
{
    _pitch = pitch;
}
qreal Dim2::vx() const
{
    return _vx;
}

void Dim2::setVx(const qreal &vx)
{
    _vx = vx;
}
qreal Dim2::vy() const
{
    return _vy;
}

void Dim2::setVy(const qreal &vy)
{
    _vy = vy;
}
qreal Dim2::vz() const
{
    return _vz;
}

void Dim2::setVz(const qreal &vz)
{
    _vz = vz;
}
qreal Dim2::vrx() const
{
    return _vrx;
}

void Dim2::setVrx(const qreal &vrx)
{
    _vrx = vrx;
}
qreal Dim2::vry() const
{
    return _vry;
}

void Dim2::setVry(const qreal &vry)
{
    _vry = vry;
}
qreal Dim2::vrz() const
{
    return _vrz;
}

void Dim2::setVrz(const qreal &vrz)
{
    _vrz = vrz;
}
qreal Dim2::mainPan() const
{
    return _mainPan;
}

void Dim2::setMainPan(const qreal &mainPan)
{
    _mainPan = mainPan;
}
qreal Dim2::mainTilt() const
{
    return _mainTilt;
}

void Dim2::setMainTilt(const qreal &mainTilt)
{
    _mainTilt = mainTilt;
}
qreal Dim2::mainFocus() const
{
    return _mainFocus;
}

void Dim2::setMainFocus(const qreal &mainFocus)
{
    _mainFocus = mainFocus;
}
qreal Dim2::mainIris() const
{
    return _mainIris;
}

void Dim2::setMainIris(const qreal &mainIris)
{
    _mainIris = mainIris;
}
qreal Dim2::verticalZoom() const
{
    return _verticalZoom;
}

void Dim2::setVerticalZoom(const qreal &verticalZoom)
{
    _verticalZoom = verticalZoom;
}
qreal Dim2::verticalFocus() const
{
    return _verticalFocus;
}

void Dim2::setVerticalFocus(const qreal &verticalFocus)
{
    _verticalFocus = verticalFocus;
}
qreal Dim2::verticalIris() const
{
    return _verticalIris;
}

void Dim2::setVerticalIris(const qreal &verticalIris)
{
    _verticalIris = verticalIris;
}
QString Dim2::event() const
{
    return _event;
}

void Dim2::setEvent(const QString &event)
{
    _event = event;
}
QString Dim2::tag() const
{
    return _tag;
}

void Dim2::setTag(const QString &tag)
{
    _tag = tag;
}




























