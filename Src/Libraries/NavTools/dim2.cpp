#include "dim2.h"

namespace nav_tools {

Dim2::Dim2(QString _args):
    m_validity_array(DIM2_FIELDS_NUMBER, '0')
{
    QStringList args_list = _args.split(";");
    int nb_args = args_list.length();
    bool ok;
    for(int no_arg = 0; no_arg < nb_args; no_arg++) {
        switch(no_arg) {
            case 0: {
                // dive
                setDiveNumber(args_list.at(no_arg).toULong(&ok));
            }
            break;
        case 1: {
            // date
            setDateTime(QDate::fromString(args_list.at(no_arg).trimmed(), "dd/MM/yyyy"));
        }
        break;
        case 2: {
            // time
            QDateTime current_date = dateTime();
            current_date.setTime(QTime::fromString(args_list.at(no_arg).trimmed(), "hh:mm:ss.zzz"));
            setDateTime(current_date);
            ok = dateTime().isValid();
        }
        break;

        case 3: {
            // src
            setSource(args_list.at(no_arg).trimmed());
            ok = !source().isEmpty();
         }
        break;

        case 4: {
            // format
            setFormat(args_list.at(no_arg).trimmed());
            ok = !format().isEmpty();
         }
        break;

        case 5: {
            // name
            setFilename(args_list.at(no_arg).trimmed());
            ok = !filename().isEmpty();
         }
        break;

        case 6: {
            // latitude
            setLatitude(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 7: {
            // longitude
            setLongitude(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 8: {
            // depth
            setDepth(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 9: {
            // altitude
            setAltitude(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 10: {
            // yaw
            setYaw(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 11: {
            // roll
            setRoll(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 12: {
            // pitch
            setPitch(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 13: {
            // vx
            setVx(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 14: {
            // vy
            setVy(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 15: {
            // vz
            setVz(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 16: {
            // vrx
            setVrx(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 17: {
            // vry
            setVry(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 18: {
            // vrz
            setVrz(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 19: {
            // main pan
            setMainPan(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 20: {
            // main tilt
            setMainTilt(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 21: {
            // main zoom
            setMainZoom(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 22: {
            // main focus
            setMainFocus(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 23: {
            // main iris
            setMainIris(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 24: {
            // vertical zoom
            setVerticalZoom(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 25: {
            // vertical focus
            setVerticalFocus(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 26: {
            // vertical iris
            setVerticalIris(args_list.at(no_arg).toDouble(&ok));
         }
        break;

        case 27: {
            // event
            setEvent(args_list.at(no_arg).trimmed());
            ok = !event().isEmpty();
         }
        break;

        case 28: {
            // tag
            setTag(args_list.at(no_arg).trimmed());
            ok = !tag().isEmpty();
         }
        break;

        case 29: {
            // comment
            setComment(args_list.at(no_arg).trimmed());
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

        if (no_arg == 0) {
                m_validity_array.replace(0, 1, flag);
        } else if (no_arg > 1) {
            m_validity_array.replace(no_arg-1, 1, flag);
        }

    }

}

QString Dim2::comment() const
{
    return m_comment;
}

void Dim2::setComment(const QString &_comment)
{
    m_comment = _comment;
}
double Dim2::mainZoom() const
{
    return m_main_zoom;
}

void Dim2::setMainZoom(const double &_main_zoom)
{
    m_main_zoom = _main_zoom;
}

quint32 Dim2::diveNumber() const
{
    return m_dive_number;
}

void Dim2::setDiveNumber(const quint32 &_dive_number)
{
    m_dive_number = _dive_number;
}
QDateTime Dim2::dateTime() const
{
    return m_date_time;
}

void Dim2::setDateTime(const QDateTime &_date_time)
{
    m_date_time = _date_time;
}

void Dim2::setDateTime(const QDate &_date)
{
    m_date_time = QDateTime(_date);
}

QString Dim2::source() const
{
    return m_source;
}

void Dim2::setSource(const QString &_source)
{
    m_source = _source;
}
QString Dim2::format() const
{
    return m_format;
}

void Dim2::setFormat(const QString &_format)
{
    m_format = _format;
}
QString Dim2::filename() const
{
    return m_filename;
}

void Dim2::setFilename(const QString &_filename)
{
    m_filename = _filename;
}
double Dim2::latitude() const
{
    return m_latitude;
}

void Dim2::setLatitude(const double &_latitude)
{
    m_latitude = _latitude;
}
double Dim2::longitude() const
{
    return m_longitude;
}

void Dim2::setLongitude(const double &_longitude)
{
    m_longitude = _longitude;
}
double Dim2::depth() const
{
    return m_depth;
}

void Dim2::setDepth(const double &_depth)
{
    m_depth = _depth;
}
double Dim2::altitude() const
{
    return m_altitude;
}

void Dim2::setAltitude(const double &_altitude)
{
    m_altitude = _altitude;
}
double Dim2::yaw() const
{
    return m_yaw;
}

void Dim2::setYaw(const double &_yaw)
{
    m_yaw = _yaw;
}
double Dim2::roll() const
{
    return m_roll;
}

void Dim2::setRoll(const double &_roll)
{
    m_roll = _roll;
}
double Dim2::pitch() const
{
    return m_pitch;
}

void Dim2::setPitch(const double &_pitch)
{
    m_pitch = _pitch;
}
double Dim2::vx() const
{
    return m_vx;
}

void Dim2::setVx(const double &_vx)
{
    m_vx = _vx;
}
double Dim2::vy() const
{
    return m_vy;
}

void Dim2::setVy(const double &_vy)
{
    m_vy = _vy;
}
double Dim2::vz() const
{
    return m_vz;
}

void Dim2::setVz(const double &_vz)
{
    m_vz = _vz;
}
double Dim2::vrx() const
{
    return m_vrx;
}

void Dim2::setVrx(const double &_vrx)
{
    m_vrx = _vrx;
}
double Dim2::vry() const
{
    return m_vry;
}

void Dim2::setVry(const double &_vry)
{
    m_vry = _vry;
}
double Dim2::vrz() const
{
    return m_vrz;
}

void Dim2::setVrz(const double &_vrz)
{
    m_vrz = _vrz;
}
double Dim2::mainPan() const
{
    return m_main_pan;
}

void Dim2::setMainPan(const double &_main_pan)
{
    m_main_pan = _main_pan;
}
double Dim2::mainTilt() const
{
    return m_main_tilt;
}

void Dim2::setMainTilt(const double &_main_tilt)
{
    m_main_tilt = _main_tilt;
}
double Dim2::mainFocus() const
{
    return m_main_focus;
}

void Dim2::setMainFocus(const double &_main_focus)
{
    m_main_focus = _main_focus;
}
double Dim2::mainIris() const
{
    return m_main_iris;
}

void Dim2::setMainIris(const double &_main_iris)
{
    m_main_iris = _main_iris;
}
double Dim2::verticalZoom() const
{
    return m_vertical_zoom;
}

void Dim2::setVerticalZoom(const double &_vertical_zoom)
{
    m_vertical_zoom = _vertical_zoom;
}
double Dim2::verticalFocus() const
{
    return m_vertical_focus;
}

void Dim2::setVerticalFocus(const double &_vertical_focus)
{
    m_vertical_focus = _vertical_focus;
}
double Dim2::verticalIris() const
{
    return m_vertical_iris;
}

void Dim2::setVerticalIris(const double &_vertical_iris)
{
    m_vertical_iris = _vertical_iris;
}
QString Dim2::event() const
{
    return m_event;
}

void Dim2::setEvent(const QString &_event)
{
    m_event = _event;
}
QString Dim2::tag() const
{
    return m_tag;
}

void Dim2::setTag(const QString &_tag)
{
    m_tag = _tag;
}

} // namespace nav_tools




























