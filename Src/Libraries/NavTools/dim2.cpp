#include "dim2.h"

namespace nav_tools {

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

        char flag[1] = {'1'};
        if (!ok) {
            flag[0] = '0';
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
double Dim2::mainZoom() const
{
    return _mainZoom;
}

void Dim2::setMainZoom(const double &mainZoom)
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
double Dim2::latitude() const
{
    return _latitude;
}

void Dim2::setLatitude(const double &latitude)
{
    _latitude = latitude;
}
double Dim2::longitude() const
{
    return _longitude;
}

void Dim2::setLongitude(const double &longitude)
{
    _longitude = longitude;
}
double Dim2::depth() const
{
    return _depth;
}

void Dim2::setDepth(const double &depth)
{
    _depth = depth;
}
double Dim2::altitude() const
{
    return _altitude;
}

void Dim2::setAltitude(const double &altitude)
{
    _altitude = altitude;
}
double Dim2::yaw() const
{
    return _yaw;
}

void Dim2::setYaw(const double &yaw)
{
    _yaw = yaw;
}
double Dim2::roll() const
{
    return _roll;
}

void Dim2::setRoll(const double &roll)
{
    _roll = roll;
}
double Dim2::pitch() const
{
    return _pitch;
}

void Dim2::setPitch(const double &pitch)
{
    _pitch = pitch;
}
double Dim2::vx() const
{
    return _vx;
}

void Dim2::setVx(const double &vx)
{
    _vx = vx;
}
double Dim2::vy() const
{
    return _vy;
}

void Dim2::setVy(const double &vy)
{
    _vy = vy;
}
double Dim2::vz() const
{
    return _vz;
}

void Dim2::setVz(const double &vz)
{
    _vz = vz;
}
double Dim2::vrx() const
{
    return _vrx;
}

void Dim2::setVrx(const double &vrx)
{
    _vrx = vrx;
}
double Dim2::vry() const
{
    return _vry;
}

void Dim2::setVry(const double &vry)
{
    _vry = vry;
}
double Dim2::vrz() const
{
    return _vrz;
}

void Dim2::setVrz(const double &vrz)
{
    _vrz = vrz;
}
double Dim2::mainPan() const
{
    return _mainPan;
}

void Dim2::setMainPan(const double &mainPan)
{
    _mainPan = mainPan;
}
double Dim2::mainTilt() const
{
    return _mainTilt;
}

void Dim2::setMainTilt(const double &mainTilt)
{
    _mainTilt = mainTilt;
}
double Dim2::mainFocus() const
{
    return _mainFocus;
}

void Dim2::setMainFocus(const double &mainFocus)
{
    _mainFocus = mainFocus;
}
double Dim2::mainIris() const
{
    return _mainIris;
}

void Dim2::setMainIris(const double &mainIris)
{
    _mainIris = mainIris;
}
double Dim2::verticalZoom() const
{
    return _verticalZoom;
}

void Dim2::setVerticalZoom(const double &verticalZoom)
{
    _verticalZoom = verticalZoom;
}
double Dim2::verticalFocus() const
{
    return _verticalFocus;
}

void Dim2::setVerticalFocus(const double &verticalFocus)
{
    _verticalFocus = verticalFocus;
}
double Dim2::verticalIris() const
{
    return _verticalIris;
}

void Dim2::setVerticalIris(const double &verticalIris)
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

} // namespace nav_tools




























