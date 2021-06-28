#ifndef NAV_TOOLS_DIM2_H_
#define NAV_TOOLS_DIM2_H_



#include <QtGlobal>
#include <QDateTime>
#include <QTime>
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace nav_tools {

// Nombre de champs (time + date = 1champ...)
#define DIM2_FIELDS_NUMBER 29

///
/// \brief Classe de représentation d'une entrée DIM2
///
/// Le format DIM2 est décrit dans le document "Description données engins V1.0.pdf" référencé 28/08/2012 – IMN/SM/12-220
///
class Dim2
{
public:
    Dim2(QString _args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &_dive_number);

    QDateTime dateTime() const;
    void setDateTime(const QDateTime &_date_time);
    void setDateTime(const QDate &_date);

    QString source() const;
    void setSource(const QString &_source);

    QString format() const;
    void setFormat(const QString &_format);

    QString filename() const;
    void setFilename(const QString &_filename);

    double latitude() const;
    void setLatitude(const double &_latitude);

    double longitude() const;
    void setLongitude(const double &_longitude);

    double depth() const;
    void setDepth(const double &_depth);

    double altitude() const;
    void setAltitude(const double &_altitude);

    double yaw() const;
    void setYaw(const double &_yaw);

    double roll() const;
    void setRoll(const double &_roll);

    double pitch() const;
    void setPitch(const double &_pitch);

    double vx() const;
    void setVx(const double &_vx);

    double vy() const;
    void setVy(const double &_vy);

    double vz() const;
    void setVz(const double &_vz);

    double vrx() const;
    void setVrx(const double &_vrx);

    double vry() const;
    void setVry(const double &_vry);

    double vrz() const;
    void setVrz(const double &_vrz);

    double mainPan() const;
    void setMainPan(const double &_main_pan);

    double mainTilt() const;
    void setMainTilt(const double &_main_tilt);

    double mainZoom() const;
    void setMainZoom(const double &_main_zoom);

    double mainFocus() const;
    void setMainFocus(const double &_main_focus);

    double mainIris() const;
    void setMainIris(const double &_main_iris);

    double verticalZoom() const;
    void setVerticalZoom(const double &_vertical_zoom);

    double verticalFocus() const;
    void setVerticalFocus(const double &_vertical_focus);

    double verticalIris() const;
    void setVerticalIris(const double &_vertical_iris);

    QString event() const;
    void setEvent(const QString &_event);

    QString tag() const;
    void setTag(const QString &_tag);

    QString comment() const;
    void setComment(const QString &_comment);

private:
    QByteArray m_validity_array;

    quint32 m_dive_number;
    QDateTime m_date_time;
    QString m_source;
    QString m_format;
    QString m_filename;
    double m_latitude;
    double m_longitude;
    double m_depth;
    double m_altitude;
    double m_yaw;
    double m_roll;
    double m_pitch;
    double m_vx;
    double m_vy;
    double m_vz;
    double m_vrx;
    double m_vry;
    double m_vrz;
    double m_main_pan;
    double m_main_tilt;
    double m_main_zoom;
    double m_main_focus;
    double m_main_iris;
    double m_vertical_zoom;
    double m_vertical_focus;
    double m_vertical_iris;
    QString m_event;
    QString m_tag;
    QString m_comment;

};

}
#endif // NAV_TOOLS_DIM2_H_
