#include "element_widget.h"
#include "assembly_graphics_scene.h"

namespace matisse {

ElementWidget::ElementWidget(QGraphicsItem *parent) :
    QObject(0),
    QGraphicsItem(parent)
{
    _bodyRect = QRect(-70, -30, 140, 60);
    _brush.setColor(Qt::white);
    //_pen.setColor(Qt::black);
    _pen.setColor(QColor::fromRgb(245, 247, 250)); // gris clair de la charte graphique (Matisse.css)
    setInputsNumber();
    setOutputsNumber();
    setAcceptHoverEvents(true);
    setFlags(ItemIsFocusable | ItemIsSelectable /*| ItemIsMovable*/);
    _buttonPressed = false;
    _showFrame = false;
    _startMove = QPointF(0,0);
    _color = QColor();
    setOrder();
}

void ElementWidget::setName(QString name) {
    _name = name;
}

void ElementWidget::clone(ElementWidget * other)
{
    if (!other) {
        return;
    }

    _inputsNumber = other -> _inputsNumber;
    _outputsNumber = other -> _outputsNumber;
    _bodyRect = other -> _bodyRect;
    _boundingRect = other -> _boundingRect;
    _brush = other -> _brush;
    _pen = other -> _pen;
    _name = other -> _name;
    _buttonPressed = other -> _buttonPressed;
    _inputsZones = other -> _inputsZones;
    _outputsZones = other -> _outputsZones;
    _inputsWidgets = other->_inputsWidgets;
    _outputsWidgets = other->_outputsWidgets;
    _eltOrder = other->_eltOrder;
    _color = other->_color;
}

QString ElementWidget::getName() {
    return _name;
}

int ElementWidget::getInputLine(QPointF pos)
{
    if (pos.isNull()) {
        return -1;
    }

    int ret = -1;

    for (int index = 0; index < _inputsZones.length(); index++) {
        QPointF posTemp = mapFromScene(pos);
        if (!_inputsZones[index].isNull()) {
            posTemp -= _inputsZones[index];
            if (posTemp.manhattanLength() < 9.0) {
                ret = index;
                break;
            }
        }
    }

    return ret;
}

int ElementWidget::getOutputLine(QPointF pos)
{
    if (pos.isNull()) {
        return -1;
    }

    int ret = -1;

    for (int index = 0; index < _outputsZones.length(); index++) {
        QPointF posTemp = mapFromScene(pos);
        if (!_outputsZones[index].isNull()) {
            posTemp -= _outputsZones[index];
            if (posTemp.manhattanLength() < 9.0) {
                ret = index;
                break;
            }
        }
    }

    return ret;
}

void ElementWidget::setColor(QColor color)
{
    _color = color;
}

QColor ElementWidget::getColor()
{
    return _color;
}

QPointF ElementWidget::getInputLinePos(quint8 inputLine)
{
    if (inputLine >= _inputsNumber) {
        return QPointF();
    }
    return mapToScene(_inputsZones[inputLine]);
}

QPointF ElementWidget::getOutputLinePos(quint8 outputLine)
{
    if (outputLine >= _outputsNumber) {
        return QPointF();
    }
    return mapToScene(_outputsZones[outputLine]);
}

bool ElementWidget::setInputWidget(quint8 noLine, ElementWidget *element)
{
    if (noLine >= _inputsNumber) {
        return false;
    }
    ElementWidget * elt = _inputsWidgets.at(noLine).data();
    if (elt) {
        delete elt;
    }
    _inputsWidgets[noLine] = element;

    return true;
}

bool ElementWidget::setOutputWidget(quint8 noLine, ElementWidget * element)
{
    if (noLine >= _outputsNumber) {
        return false;
    }
    ElementWidget * elt = _outputsWidgets.at(noLine).data();
    if (elt) {
        delete elt;
    }
    _outputsWidgets[noLine] = element;

    return true;
}

ElementWidget * ElementWidget::getInputWidget(quint8 noLine)
{
    if (noLine >= _inputsNumber) {
        return 0;
    }

    return  _inputsWidgets.at(noLine);
}

ElementWidget * ElementWidget::getOutputWidget(quint8 noLine)
{
    if (noLine >= _outputsNumber) {
        return 0;
    }

    return  _outputsWidgets.at(noLine);
}

void ElementWidget::computeBoundingRect()
{
    _boundingRect = _bodyRect;
    if (_inputsNumber > 0) {
        _boundingRect.setTop(_bodyRect.top()-38);
    }
    if (_outputsNumber > 0) {
        _boundingRect.setBottom(_bodyRect.bottom() + 44);
    }
}

QIcon ElementWidget::getIcon()
{
    QRectF eltRect(_boundingRect);
    eltRect.adjust(0, 0, 1, 0);
    QPixmap image(eltRect.width(), eltRect.height());
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setPen(_pen);
    drawSymbol(&painter, true);

    return QIcon(image);
}

void ElementWidget::setInputsNumber(int number)
{
    _inputsNumber = number;

    _inputsWidgets.resize(number);
//    _inputsWidgets.clear();

    int procWidth = _bodyRect.width();
    //int xOrig = _processorRect.x();
    int xOrig = _bodyRect.left();
    int yOrig = _bodyRect.top();
    int deltaIn = procWidth / (_inputsNumber + 1);

    for (int index = 1; index <= _inputsNumber; index++) {
        _inputsZones << QPointF(xOrig + deltaIn * index, yOrig - 36);
    }

    computeBoundingRect();
}

void ElementWidget::setOutputsNumber(int number)
{
    _outputsNumber = number;

    _outputsWidgets.resize(number);
    //_outputsWidgets.clear();

    int srcWidth = _bodyRect.width();
    //int xOrig = _processorRect.x() /*+ 5*/;
    int xOrig = _bodyRect.left();
    int yOrig = _bodyRect.top();
    int deltaOut = srcWidth / (_outputsNumber + 1);

    for (int index = 1; index <= _outputsNumber; index++) {
        _outputsZones << QPointF(xOrig + deltaOut * index, yOrig + _bodyRect.height() + 36);
    }

    computeBoundingRect();
}

void ElementWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setPen(_pen);
    painter->setBrush(_brush);

    QFont font = painter->font();
    font.setPointSize(font.pointSize()-1);
    painter->setFont(font);

    drawSymbol(painter);

    // pour debug _showFrame = true
    //_showFrame = true;
    if (_showFrame) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(Qt::red);
        painter->drawRect(_boundingRect);
    }

    painter->restore();

}

QRectF ElementWidget::boundingRect() const
{
    return _boundingRect;

    // return _rect;
}

void ElementWidget::setOrder(qint8 order)
{
    _eltOrder = order;
}

qint8 ElementWidget::getOrder()
{
    return _eltOrder;
}

void ElementWidget::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event)
    //event->setAccepted(event->mimeData()
}

//void ElementWidget::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
//{
//    qDebug() << "Hover move...";
//    if (getOutputLine(event->scenePos()) > -1) {
//        qDebug() << "openHand";
//        scene()->views().at(0)->setCursor(Qt::OpenHandCursor);
//    } else {

//    }
//}

} // namespace matisse
