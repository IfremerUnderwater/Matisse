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

    qreal latitude() const;
    void setLatitude(const qreal &latitude);

    qreal longitude() const;
    void setLongitude(const qreal &longitude);

    qreal depth() const;
    void setDepth(const qreal &depth);

    qreal altitude() const;
    void setAltitude(const qreal &altitude);

    qreal yaw() const;
    void setYaw(const qreal &yaw);

    qreal roll() const;
    void setRoll(const qreal &roll);

    qreal pitch() const;
    void setPitch(const qreal &pitch);

    qreal vx() const;
    void setVx(const qreal &vx);

    qreal vy() const;
    void setVy(const qreal &vy);

    qreal vz() const;
    void setVz(const qreal &vz);

    qreal vrx() const;
    void setVrx(const qreal &vrx);

    qreal vry() const;
    void setVry(const qreal &vry);

    qreal vrz() const;
    void setVrz(const qreal &vrz);

    qreal mainPan() const;
    void setMainPan(const qreal &mainPan);

    qreal mainTilt() const;
    void setMainTilt(const qreal &mainTilt);

    qreal mainZoom() const;
    void setMainZoom(const qreal &mainZoom);

    qreal mainFocus() const;
    void setMainFocus(const qreal &mainFocus);

    qreal mainIris() const;
    void setMainIris(const qreal &mainIris);

    qreal verticalZoom() const;
    void setVerticalZoom(const qreal &verticalZoom);

    qreal verticalFocus() const;
    void setVerticalFocus(const qreal &verticalFocus);

    qreal verticalIris() const;
    void setVerticalIris(const qreal &verticalIris);

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
    qreal _latitude;
    qreal _longitude;
    qreal _depth;
    qreal _altitude;
    qreal _yaw;
    qreal _roll;
    qreal _pitch;
    qreal _vx;
    qreal _vy;
    qreal _vz;
    qreal _vrx;
    qreal _vry;
    qreal _vrz;
    qreal _mainPan;
    qreal _mainTilt;
    qreal _mainZoom;
    qreal _mainFocus;
    qreal _mainIris;
    qreal _verticalZoom;
    qreal _verticalFocus;
    qreal _verticalIris;
    QString _event;
    QString _tag;
    QString _comment;

};

}
#endif // DIM2_H
