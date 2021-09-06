#ifndef MATISSE_LIVE_PROCESS_WHEEL_H_
#define MATISSE_LIVE_PROCESS_WHEEL_H_

#include <QtDebug>
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <qmath.h>

namespace matisse {

enum eWheelState {
    INACTIVE,
    RUNNING,
    FROZEN
};



class LiveProcessWheel : public QWidget
{
    Q_OBJECT
public:
    explicit LiveProcessWheel(QWidget *_parent = 0);
    ~LiveProcessWheel();

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *_evt);

private slots:
    void sl_newHour();

public slots:
    void sl_processRunning();
    void sl_processStopped();
    void sl_processFrozen();
    void sl_updateWheelColors(QString _colors);

private:
    void selectColor(QPainter & _painter, int _hour);
    void inactivateWheel();

    QPointF* m_center;
    QColor* m_inactive_color;
    QColor* m_active_color;
    QColor* m_leading_active_color;

    double m_wheel_radius;
    double m_inner_radius;
    double m_ray_width;
    quint32 m_ms_for_one_hour_clock_turn;
    quint8 m_current_hour;
    bool m_first_round;

    QTimer* m_clock_timer;
    eWheelState m_state;
    QColor** m_current_tick_colors;
};

} // namespace matisse

#endif // MATISSE_LIVE_PROCESS_WHEEL_H_
