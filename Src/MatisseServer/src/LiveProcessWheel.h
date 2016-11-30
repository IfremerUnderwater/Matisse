#ifndef LIVEPROCESSWHEEL_H
#define LIVEPROCESSWHEEL_H

#include <QtDebug>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <qmath.h>

namespace MatisseServer {

enum WheelState {
    INACTIVE,
    RUNNING,
    FROZEN
};



class LiveProcessWheel : public QWidget
{
    Q_OBJECT
public:
    explicit LiveProcessWheel(QWidget *parent = 0);
    ~LiveProcessWheel();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *evt);

private slots:
    void slot_newHour();

public slots:
    void slot_processRunning();
    void slot_processStopped();
    void slot_processFrozen();
    void slot_updateWheelColors(QString colors);

private:
//    void initWheel();
    void selectColor(QPainter & painter, int hour);
    void inactivateWheel();

    QPointF* _center;
    QColor* inactiveColor;
    QColor* activeColor;
    QColor* leadingActiveColor;

    double _wheelRadius;
    double _innerRadius;
    double _rayWidth;
    quint32 _msForOneHourClockTurn;
    quint8 _currentHour;
    bool _firstRound;

    QTimer* _clockTimer;
    WheelState _state;
    QColor** _currentTickColors;
    //    bool _wheelInitialized;
};

}

#endif // LIVEPROCESSWHEEL_H
