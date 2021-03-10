#ifndef DIM2_H
#define DIM2_H



#include <QtGlobal>
#include <QDateTime>
#include <QTime>
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace MatisseCommon {

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
    Dim2(QString args);

    quint32 diveNumber() const;
    void setDiveNumber(const quint32 &diveNumber);

    QDateTime dateTime() const;
    void setDateTime(const QDateTime &dateTime);
    void setDateTime(const QDate &date);

    QString source() const;
    void setSource(const QString &source);

    QString format() const;
    void setFormat(const QString &format);

    QString filename() const;
    void setFilename(const QString &filename);

    double latitude() const;
    void setLatitude(const double &latitude);

    double longitude() const;
    void setLongitude(const double &longitude);

    double depth() const;
    void setDepth(const double &depth);

    double altitude() const;
    void setAltitude(const double &altitude);

    double yaw() const;
    void setYaw(const double &yaw);

    double roll() const;
    void setRoll(const double &roll);

    double pitch() const;
    void setPitch(const double &pitch);

    double vx() const;
    void setVx(const double &vx);

    double vy() const;
    void setVy(const double &vy);

    double vz() const;
    void setVz(const double &vz);

    double vrx() const;
    void setVrx(const double &vrx);

    double vry() const;
    void setVry(const double &vry);

    double vrz() const;
    void setVrz(const double &vrz);

    double mainPan() const;
    void setMainPan(const double &mainPan);

    double mainTilt() const;
    void setMainTilt(const double &mainTilt);

    double mainZoom() const;
    void setMainZoom(const double &mainZoom);

    double mainFocus() const;
    void setMainFocus(const double &mainFocus);

    double mainIris() const;
    void setMainIris(const double &mainIris);

    double verticalZoom() const;
    void setVerticalZoom(const double &verticalZoom);

    double verticalFocus() const;
    void setVerticalFocus(const double &verticalFocus);

    double verticalIris() const;
    void setVerticalIris(const double &verticalIris);

    QString event() const;
    void setEvent(const QString &event);

    QString tag() const;
    void setTag(const QString &tag);

    QString comment() const;
    void setComment(const QString &comment);

private:
    QByteArray _validityArray;

    quint32 _diveNumber;
    QDateTime _dateTime;
    QString _source;
    QString _format;
    QString _filename;
    double _latitude;
    double _longitude;
    double _depth;
    double _altitude;
    double _yaw;
    double _roll;
    double _pitch;
    double _vx;
    double _vy;
    double _vz;
    double _vrx;
    double _vry;
    double _vrz;
    double _mainPan;
    double _mainTilt;
    double _mainZoom;
    double _mainFocus;
    double _mainIris;
    double _verticalZoom;
    double _verticalFocus;
    double _verticalIris;
    QString _event;
    QString _tag;
    QString _comment;

};

}
#endif // DIM2_H
