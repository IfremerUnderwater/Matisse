#ifndef ELEMENTWIDGET_H
#define ELEMENTWIDGET_H

#include <QObject>
#include <QColor>
#include <QPalette>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFont>
#include <QRectF>
#include <QPoint>
#include <QPointF>
#include <QGraphicsItem>
#include <QIcon>
#include <QGraphicsSceneMouseEvent>
#include <QDrag>
#include <QLabel>
#include <QMimeData>
#include <QPointer>
#include <QMetaObject>
#include <QtDebug>

namespace MatisseServer {
enum UserItemType { ElementType = QGraphicsItem::UserType +1,
        ParametersType,
        SourceType,
        ProcessorType,
        DestinationType,
        PipeType};


class ElementWidget : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum { Type = ElementType};

    explicit ElementWidget(QGraphicsItem *parent = 0);
    QIcon getIcon();
    virtual void setInputsNumber(int number = 0);
    virtual void setOutputsNumber(int number = 0);
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget=0);
    QRectF boundingRect() const;
    void setOrder(qint8 order = -128);
    qint8 getOrder();

/*
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
*/
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
//    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);

    void setName(QString name);
    virtual void clone(ElementWidget * other);
    //virtual QRectF getRect() = 0;

    virtual void drawSymbol(QPainter *painter, bool forIcon = false) = 0;
   // virtual ElementWidget * clone() const =0;

    QString getName();
    virtual int type() const { return ElementType;}

    int getInputLine(QPointF pos);
    int getOutputLine(QPointF pos);
    void setColor(QColor color);
    QColor getColor();

    QPointF getInputLinePos(quint8 inputLine);
    QPointF getOutputLinePos(quint8 outputLine);

    bool setInputWidget(quint8 noLine, ElementWidget * element = 0);
    bool setOutputWidget(quint8 noLine, ElementWidget * element = 0);
    ElementWidget * getInputWidget(quint8 noLine);
    ElementWidget * getOutputWidget(quint8 noLine);

protected:
    quint16 _inputsNumber;
    quint16 _outputsNumber;
    QRect _bodyRect;
    QRectF _boundingRect;
    QBrush _brush;
    QPen _pen;
    QString _name;
    bool _buttonPressed;
    bool _showFrame;
    QPointF _startMove;
    QPointF _endMove;
    QList<QPointF> _inputsZones;
    QList<QPointF> _outputsZones;
    QVector<QPointer<ElementWidget> > _inputsWidgets;
    QVector<QPointer<ElementWidget> > _outputsWidgets;
    qint8 _eltOrder;
    QColor _color;

    void computeBoundingRect();

signals:

public slots:

};
}

#endif // ELEMENTWIDGET_H
