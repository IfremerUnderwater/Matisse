#include "PipeWidget.h"

using namespace MatisseServer;

int PipeWidget::_currentColorIndex = 0;
QList<QRgb> PipeWidget::_colorsList = QList<QRgb>();

PipeWidget::PipeWidget(QGraphicsItem *parent):ElementWidget(parent),
    _startElt(0),
    _tempStartElt(0),
    _endElt(0),
    _startLine(-1),
    _endLine(-1)
{
    fillColorsList();
    qDebug() << "CONSTRUCTION PIPEWIDGET";
    setGeometry(0, 0);
    setZValue(1.0);
}

bool PipeWidget::hasStart() {
    return (_tempStartElt != 0);
}

void PipeWidget::clear()
{
    _startElt = 0;
    _tempStartElt = 0;
    _endElt = 0;
    _startLine = -1;
    _endLine = -1;
    setGeometry(0, 0);
    setVisible(false);
}

void PipeWidget::fillColorsList() {
    if (!_colorsList.isEmpty()) {
        return;
    }
    for (int index =4; index < 19; index++) {
        _colorsList.append(QColor(static_cast<Qt::GlobalColor>(index)).rgba());
    }
    _currentColorIndex = 0;
}

QRgb PipeWidget::getNextColor()
{
    _currentColorIndex++;
    if (_currentColorIndex == _colorsList.size()) {
        _currentColorIndex = 0;
    }
    return _colorsList[_currentColorIndex];
}

void PipeWidget::setCurrentColor(QRgb color)
{
    int index = _colorsList.indexOf(color);
    _currentColorIndex = qMax(0, index);
}

void PipeWidget::drawSymbol(QPainter *painter, bool forIcon)
{
    /*
    if ((!_sourceIsValid) && (!_destinationIsValid)) {
        qDebug() << "NOK!!!";
        return;
    }
*/
    // connecteur sup
    //qDebug() << "DRAW PIPE" << this;
    painter->setPen(_color);
    painter->setBrush(QBrush(_color));
    QRect src(_x - 6, _y - 6, 12, 12);
    QRect dst(_x + _deltaXt + _deltaXb -6, _y + _deltaYt + _deltaY + _deltaYb -6, 12, 12);

    painter->drawArc(src,-180*16, 180*16);
    // pipe sup
    // horizontal pipe sup
    // vertical pipe
    // horizontal pipe inf
    // pipe inf

    /*QPolygon openedPolygon;
    openedPolygon << QPoint(_x, _y +6 ) << QPoint( _x, _y + _deltaYt)
                  << QPoint(_x + _deltaXt, _y + _deltaYt)
                  << QPoint(_x + _deltaXt, _y + _deltaYt + _deltaY)
                  << QPoint(_x + _deltaXt + _deltaXb, _y + _deltaYt + _deltaY)
                  << QPoint(_x + _deltaXt + _deltaXb, _y + _deltaYt + _deltaY + _deltaYb);

    painter->drawPolyline(openedPolygon);*/

    // Connect modules with bezier curve
    QPainterPath myPath;
    int verticalDist = _deltaYt + _deltaY + _deltaYb -6;
    myPath.moveTo(_x, _y +6);
    myPath.cubicTo(_x, _y + verticalDist/2.0 ,
                   _x + _deltaXt + _deltaXb, _y + _deltaYt + _deltaY + _deltaYb -verticalDist/2.0,
                   _x + _deltaXt + _deltaXb , _y + _deltaYt + _deltaY + _deltaYb -6);
    //painter->brush().setColor(Qt::transparent);
    painter->setBrush(QColor(Qt::transparent));
    painter->drawPath(myPath);

    // connecteur inf
    painter->drawEllipse(dst);

}

void PipeWidget::setGeometry(int x, int y, int deltaYt, int deltaXt, int deltaY, int deltaXb, int deltaYb, QColor color)
{
    _x = x;
    _y = y;
    _deltaYt = deltaYt;
    _deltaXt = deltaXt;
    _deltaY = deltaY;
    _deltaXb = deltaXb;
    _deltaYb = deltaYb;

    setColor(color);

}

void PipeWidget::setColor(QColor color)
{
    if (!color.isValid()) {
        //_color = QColor(Qt::black);
        _color = QColor::fromRgb(245, 247, 250); // gris clair de la charte graphique (Matisse.css)
    } else {
        _color = color;
    }
    //update();
}

void  PipeWidget::setStart(bool temp, ElementWidget * src, int srcLine) {
    //qDebug() << "Set start..." << temp << int(src) << srcLine;
    if (temp) {
        _tempStartElt = src;
    } else {
        _startElt = src;
    }
    _startLine = srcLine;

    if (src && (srcLine > -1)) {
        qDebug() << "src dump";
        src->dumpObjectInfo();
        qDebug() << "Line" << srcLine;
        //setVisible(true);
        QPointF srcPos = mapFromScene(src->getOutputLinePos(srcLine));
        setGeometry(srcPos.x(), srcPos.y());
        if (!temp) {
            src->setOutputWidget(srcLine, this);
        }
        //_x = srcPos.x();
        //_y = srcPos.y();
       // prepareGeometryChange();
        _boundingRect = QRectF(_x-6, _y-6, 12, 12);
        //update();
    } else {
        qDebug() << "Sortie setStart 0";
        //setVisible(false);
    }
}

void PipeWidget::setEnd(ElementWidget * dest, int destLine)
{
    _endElt = dest;
    _endLine = destLine;
    if (dest && (destLine > -1)) {
        setStart(false, _tempStartElt, _startLine);
        toDestinationPos(dest->getInputLinePos(destLine));
        dest->setInputWidget(destLine, this);
        //setVisible(true);
        //update();
    }
}

ElementWidget *PipeWidget::getStartElement()
{
    return _startElt;
}

ElementWidget *PipeWidget::getTempStartElement()
{
    return _tempStartElt;
}

ElementWidget *PipeWidget::getEndElement()
{
    return _endElt;
}

int PipeWidget::getStartElementLine()
{
    return _startLine;
}

int PipeWidget::getEndElementLine()
{
    return _endLine;
}

void PipeWidget::refreshConnections()
{
    setStart(true, _startElt, _startLine);
    setEnd(_endElt, _endLine);
}

void PipeWidget::fromSourcePos(QPointF srcPos)
{
    srcPos = mapFromScene(srcPos);
    _x = srcPos.x();
    _y = srcPos.y();
    qDebug() << "from.............................................................";
    //update();
}

void PipeWidget::toDestinationPos(QPointF dstPos)
{
    dstPos = mapFromScene(dstPos);
    int dstX = dstPos.x();
    int dstY = dstPos.y();

    bool bigStep = false;
    if (_startElt && _endElt) {
        int startOrder = _startElt->getOrder();
        int endOrder = _endElt->getOrder();
        int startXPos = _startElt->getOutputLinePos(_startLine).x();
        int startYPos = _startElt->getOutputLinePos(_startLine).y();
        int endXPos = _endElt->getInputLinePos(_endLine).x();
        int endYPos = _endElt->getInputLinePos(_endLine).y();
        //int deltaPos = _endElt->scenePos().y() - _start

        if ((endOrder - startOrder) != 1) {
            bigStep = true;
            _deltaYt = _startLine * 4 + 10;
            _deltaYb = _endLine * 4 + 10;
            _deltaXt = startXPos - 200 + startOrder * 20 + _startLine * 5;
            _deltaXb = endXPos - startXPos - _deltaXt;
            _deltaY = endYPos - startYPos - _deltaYt - _deltaYb;

            // calcul bounding rect...
        }
    }
    if (!bigStep) {
        _deltaYt = qBound(-10, dstY -_y, 10);
        _deltaYb = qBound(-10, dstY -_y - _deltaYb, 10);
        _deltaY = dstPos.y() - (_y + _deltaYt + _deltaYb);
        _deltaXt = (dstX - _x)/2;
        _deltaXb = dstX - _deltaXt -_x;
    }

    //setVisible(true);
    if (_startElt) {
        double leftX = qMin(_x + _deltaXt, _x + _deltaXt + _deltaXb);
        double rightX = qMax(_x + _deltaXt, _x + _deltaXt + _deltaXb);
        QPointF locSrcPos = mapFromScene(_startElt->getOutputLinePos(_startLine));
        QPointF locDstPos = mapFromScene(dstPos);
        prepareGeometryChange();
        _boundingRect = QRectF(locSrcPos.x()-6, locSrcPos.y()-6, 12, 12);
        _boundingRect = _boundingRect.united(QRectF(locDstPos.x(), locDstPos.y(), 6, 6));
        _boundingRect = _boundingRect.united(QRectF(leftX, locSrcPos.y(), rightX - leftX, 1));
    }
    // _boundingRect = _boundingRect.united(dst);

    //    update();
}

void PipeWidget::clone(PipeWidget *other)
{
    ElementWidget::clone(other);

    _color = other->_color;
    _x = other->_x;
    _y = other->_y;
    _deltaYt = other->_deltaYt;
    _deltaXt = other->_deltaXt;
    _deltaY = other->_deltaY;
    _deltaXb = other->_deltaXb;
    _deltaYb = other->_deltaYb;
    _startElt = other->_startElt;
    _tempStartElt = other->_tempStartElt;
    _endElt = other->_endElt;
    _startLine = other->_startLine;
    _endLine = other->_endLine;
}
