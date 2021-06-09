#include "live_process_wheel.h"
#include "graphical_charter.h"

namespace matisse {

LiveProcessWheel::LiveProcessWheel(QWidget *_parent) :
    QWidget(_parent),
    m_ms_for_one_hour_clock_turn(80),
    m_current_hour(0),
    m_first_round(true),
    m_state(INACTIVE)
{
    GraphicalCharter &graph_charter = GraphicalCharter::instance();

    m_wheel_radius = (double)graph_charter.dpiScaled(25);
    m_inner_radius = (double)graph_charter.dpiScaled(13);
    m_ray_width = (double)graph_charter.dpiScaled(2);

    double center = (double)graph_charter.dpiScaled(30);
    m_center = new QPointF(center, center);

    /* widget is not aware of graphical chart, using default colors */
    m_inactive_color = new QColor(Qt::gray);
    m_active_color = new QColor(Qt::blue);
    m_leading_active_color = new QColor(Qt::green);

    /* init ticks color tabs */
    m_current_tick_colors = new QColor*[12];

    inactivateWheel();

    m_clock_timer = new QTimer(this);
    connect(m_clock_timer, SIGNAL(timeout()), this, SLOT(sl_newHour()));
}

LiveProcessWheel::~LiveProcessWheel()
{
    delete m_center;

    m_clock_timer->stop();
    delete m_clock_timer;

    for (int i = 0 ; i < 12 ; i++) {
        m_current_tick_colors[i] = NULL;
    }
    delete[] m_current_tick_colors;

    delete m_inactive_color;
    delete m_active_color;
    delete m_leading_active_color;
}

void LiveProcessWheel::selectColor(QPainter &_painter, int _hour)
{
    QColor tick_color = QColor(*m_current_tick_colors[_hour]);

    _painter.setBrush(tick_color);
    _painter.setPen(tick_color);
}

void LiveProcessWheel::paintEvent(QPaintEvent *_evt)
{
    Q_UNUSED(_evt)

    double ray_half_width = m_ray_width / 2;
    double ray_length = m_wheel_radius - m_inner_radius;

    // define clock ticks
    QRectF tick0 = QRectF(0 - ray_half_width, 0 - m_wheel_radius, m_ray_width, ray_length);

    QTransform transform1;
    transform1.rotate(30);
    QPolygonF tick1 = transform1.map(tick0);

    QTransform transform2;
    transform2.rotate(60);
    QPolygonF tick2 = transform2.map(tick0);

    QTransform transform3;
    transform3.rotate(90);
    QPolygonF tick3 = transform3.map(tick0);

    QTransform transform4;
    transform4.rotate(120);
    QPolygonF tick4 = transform4.map(tick0);

    QTransform transform5;
    transform5.rotate(150);
    QPolygonF tick5 = transform5.map(tick0);

    QTransform transform6;
    transform6.rotate(180);
    QPolygonF tick6 = transform6.map(tick0);

    QTransform transform7;
    transform7.rotate(210);
    QPolygonF tick7 = transform7.map(tick0);

    QTransform transform8;
    transform8.rotate(240);
    QPolygonF tick8 = transform8.map(tick0);

    QTransform transform9;
    transform9.rotate(270);
    QPolygonF tick9 = transform9.map(tick0);

    QTransform transform10;
    transform10.rotate(300);
    QPolygonF tick10 = transform10.map(tick0);

    QTransform transform11;
    transform11.rotate(330);
    QPolygonF tick11 = transform11.map(tick0);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // translate wheel to the center of the widget
    painter.translate(m_center->x(), m_center->y());

    // trace clock ticks
    selectColor(painter, 0);
    painter.drawRect(tick0);

    selectColor(painter, 1);
    painter.drawPolygon(tick1);

    selectColor(painter, 2);
    painter.drawPolygon(tick2);

    selectColor(painter, 3);
    painter.drawPolygon(tick3);

    selectColor(painter, 4);
    painter.drawPolygon(tick4);

    selectColor(painter, 5);
    painter.drawPolygon(tick5);

    selectColor(painter, 6);
    painter.drawPolygon(tick6);

    selectColor(painter, 7);
    painter.drawPolygon(tick7);

    selectColor(painter, 8);
    painter.drawPolygon(tick8);

    selectColor(painter, 9);
    painter.drawPolygon(tick9);

    selectColor(painter, 10);
    painter.drawPolygon(tick10);

    selectColor(painter, 11);
    painter.drawPolygon(tick11);
}

void LiveProcessWheel::sl_newHour()
{
    if (m_first_round && m_current_hour == 12) {
        m_first_round = false;
    }

    m_current_hour = m_current_hour % 12;

    qint8 trailing_tick = m_current_hour - 7;
    /* At first round, trailing tick is not before noon */
    if (m_first_round) {
        if (trailing_tick < 0) {
            trailing_tick = 0;
        }
    }

    for (quint8 i = 0; i < 12 ; i++ ) {

        if (i == m_current_hour) {
            m_current_tick_colors[i] = m_leading_active_color;
        } else {
            qint8 ofs_i = i;

            /* offset index, so that m_current_hour is the highest active value */
            if (i > m_current_hour) {
                ofs_i = i - 12;
            }

            if (ofs_i >= trailing_tick) {
                m_current_tick_colors[i] = m_active_color;
            } else {
                m_current_tick_colors[i] = m_inactive_color;
            }
        }
    }

    m_current_hour++;
    repaint();
}

void LiveProcessWheel::inactivateWheel()
{
    for (quint8 i = 0 ; i < 12 ; i++) {
        m_current_tick_colors[i] = m_inactive_color;
    }
}

void LiveProcessWheel::sl_processRunning()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(m_state).arg(QString::number(RUNNING));

    // Reset clock hour only if previous state was inactive
    // (if previous state was frozen, then start again from same hour)
    if (m_state == INACTIVE) {
        m_current_hour = 0;
        m_first_round = true;
    }
    m_clock_timer->start(m_ms_for_one_hour_clock_turn);
    m_state = RUNNING;
    setToolTip(tr("Process running..."));
}

void LiveProcessWheel::sl_processStopped()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(m_state).arg(QString::number(INACTIVE));
    if (m_clock_timer->isActive()) {
        m_clock_timer->stop();
    }
    m_state = INACTIVE;
    inactivateWheel();
    setToolTip(tr("No process running"));
    repaint();
}

void LiveProcessWheel::sl_processFrozen()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(m_state).arg(QString::number(FROZEN));
    m_clock_timer->stop();
    m_state = FROZEN;
    setToolTip(tr("Frozen processing"));
}

void LiveProcessWheel::sl_updateWheelColors(QString _colors)
{
    if (_colors.isEmpty()) {
        qCritical() << "Wheel colors not provided";
        return;
    }

    QStringList colors_list = _colors.split("-");
    if (colors_list.size() < 3) {
        qCritical() << QString("%1 colors were provided, but 3 are expected").arg(colors_list.size());
        return;
    }

    if (m_clock_timer->isActive()) {
        qWarning() << "Wheel colors update event received while wheel is active, stopping...";
        m_clock_timer->stop();
    }

    delete m_inactive_color;
    delete m_active_color;
    delete m_leading_active_color;

    m_inactive_color = new QColor(colors_list.at(0));
    m_active_color = new QColor(colors_list.at(1));
    m_leading_active_color = new QColor(colors_list.at(2));

    inactivateWheel();
    repaint();
}

} // namespace matisse
