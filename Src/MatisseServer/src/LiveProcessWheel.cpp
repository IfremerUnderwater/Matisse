#include "LiveProcessWheel.h"

using namespace MatisseServer;

LiveProcessWheel::LiveProcessWheel(QWidget *parent) : QWidget(parent),
    _wheelRadius(25.0), _innerRadius(13.0), _rayWidth(2.0), _state(INACTIVE),
    _currentHour(0), _firstRound(true), _msForOneHourClockTurn(80)
{
    _center = new QPointF(30.0, 30.0);

    /* widget is not aware of graphical chart, using default colors */
    inactiveColor = new QColor(Qt::gray);
    activeColor = new QColor(Qt::blue);
    leadingActiveColor = new QColor(Qt::green);

    /* init ticks color tabs */
    _currentTickColors = new QColor*[12];

    inactivateWheel();

    _clockTimer = new QTimer(this);
    connect(_clockTimer, SIGNAL(timeout()), this, SLOT(slot_newHour()));
}

LiveProcessWheel::~LiveProcessWheel()
{
    delete _center;

    _clockTimer->stop();
    delete _clockTimer;

    for (int i = 0 ; i < 12 ; i++) {
        _currentTickColors[i] = NULL;
    }
    delete[] _currentTickColors;

    delete inactiveColor;
    delete activeColor;
    delete leadingActiveColor;
}

void LiveProcessWheel::selectColor(QPainter &painter, int hourTick)
{
    QColor tickColor = QColor(*_currentTickColors[hourTick]);

    painter.setBrush(tickColor);
    painter.setPen(tickColor);
}

void LiveProcessWheel::paintEvent(QPaintEvent *evt)
{
    double rayHalfWidth = _rayWidth / 2;
    double rayLength = _wheelRadius - _innerRadius;

    // define clock ticks
    QRectF tick0 = QRectF(0 - rayHalfWidth, 0 - _wheelRadius, _rayWidth, rayLength);

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
    painter.translate(_center->x(), _center->y());

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

void LiveProcessWheel::slot_newHour()
{
    if (_firstRound && _currentHour == 12) {
        _firstRound = false;
    }

    _currentHour = _currentHour % 12;

    qint8 trailingTick = _currentHour - 7;
    /* At first round, trailing tick is not before noon */
    if (_firstRound) {
        if (trailingTick < 0) {
            trailingTick = 0;
        }
    }

    for (quint8 i = 0; i < 12 ; i++ ) {

        if (i == _currentHour) {
            _currentTickColors[i] = leadingActiveColor;
        } else {
            qint8 ofs_i = i;

            /* offset index, so that _currentHour is the highest active value */
            if (i > _currentHour) {
                ofs_i = i - 12;
            }

            if (ofs_i >= trailingTick) {
                _currentTickColors[i] = activeColor;
            } else {
                _currentTickColors[i] = inactiveColor;
            }
        }
    }

    _currentHour++;
    repaint();
}

void LiveProcessWheel::inactivateWheel()
{
    for (quint8 i = 0 ; i < 12 ; i++) {
        _currentTickColors[i] = inactiveColor;
    }
}

void LiveProcessWheel::slot_processRunning()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(_state).arg(QString::number(RUNNING));

    // Reset clock hour only if previous state was inactive
    // (if previous state was frozen, then start again from same hour)
    if (_state == INACTIVE) {
        _currentHour = 0;
        _firstRound = true;
    }
    _clockTimer->start(_msForOneHourClockTurn);
    _state = RUNNING;
    setToolTip(tr("Traitement en cours..."));
}

void LiveProcessWheel::slot_processStopped()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(_state).arg(QString::number(INACTIVE));
    if (_clockTimer->isActive()) {
        _clockTimer->stop();
    }
    _state = INACTIVE;
    inactivateWheel();
    setToolTip(tr("Pas de traitement en cours"));
    repaint();
}

void LiveProcessWheel::slot_processFrozen()
{
    qDebug() << QString("Live process wheel state change %1 -> %2").arg(_state).arg(QString::number(FROZEN));
    _clockTimer->stop();
    _state = FROZEN;
    setToolTip(tr("Traitement fige"));
}

void LiveProcessWheel::slot_updateWheelColors(QString colors)
{
    if (colors.isEmpty()) {
        qCritical() << "Wheel colors not provided";
        return;
    }

    QStringList colorsList = colors.split("-");
    if (colorsList.size() < 3) {
        qCritical() << QString("%1 colors were provided, but 3 are expected").arg(colorsList.size());
        return;
    }

    if (_clockTimer->isActive()) {
        qWarning() << "Wheel colors update event received while wheel is active, stopping...";
        _clockTimer->stop();
    }

    delete inactiveColor;
    delete activeColor;
    delete leadingActiveColor;

    inactiveColor = new QColor(colorsList.at(0));
    activeColor = new QColor(colorsList.at(1));
    leadingActiveColor = new QColor(colorsList.at(2));

    inactivateWheel();
    repaint();
}
