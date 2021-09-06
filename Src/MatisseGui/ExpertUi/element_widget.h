#ifndef MATISSE_ELEMENT_WIDGET_H_
#define MATISSE_ELEMENT_WIDGET_H_

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

namespace matisse {

enum eUserItemType { ELEMENT_TYPE = QGraphicsItem::UserType +1,
        SOURCE_TYPE,
        PROCESSOR_TYPE,
        DESTINATION_TYPE,
        PIPE_TYPE};


class ElementWidget : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum { Type = ELEMENT_TYPE};

    explicit ElementWidget(QGraphicsItem *_parent = 0);
    QIcon getIcon();
    virtual void setInputsNumber(int _number = 0);
    virtual void setOutputsNumber(int _number = 0);
    void paint(QPainter * _painter, const QStyleOptionGraphicsItem * _option, QWidget * _widget=0);
    QRectF boundingRect() const;
    void setOrder(qint8 _order = -128);
    qint8 getOrder();

    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *_event);

    void setName(QString _name);
    virtual void clone(ElementWidget * _other);

    virtual void drawSymbol(QPainter *_painter, bool _for_icon = false) = 0;

    QString getName();
    virtual int type() const { return ELEMENT_TYPE;}

    int getInputLine(QPointF _pos);
    int getOutputLine(QPointF _pos);
    void setColor(QColor _color);
    QColor getColor();

    QPointF getInputLinePos(quint8 _input_line);
    QPointF getOutputLinePos(quint8 _output_line);

    bool setInputWidget(quint8 _no_line, ElementWidget * _element = 0);
    bool setOutputWidget(quint8 _no_line, ElementWidget * _element = 0);
    ElementWidget * getInputWidget(quint8 _no_line);
    ElementWidget * getOutputWidget(quint8 _no_line);

protected:
    quint16 m_inputs_number;
    quint16 m_outputs_number;
    QRect m_body_rect;
    QRectF m_bounding_rect;
    QBrush m_brush;
    QPen m_pen;
    QString m_name;
    bool m_button_pressed;
    bool m_show_frame;
    QPointF m_start_move;
    QPointF m_end_move;
    QList<QPointF> m_inputs_zones;
    QList<QPointF> m_outputs_zones;
    QVector<QPointer<ElementWidget> > m_inputs_widgets;
    QVector<QPointer<ElementWidget> > m_outputs_widgets;
    qint8 m_elt_order;
    QColor m_color;

    void computeBoundingRect();

signals:

public slots:

};

} // namespace matisse

#endif // MATISSE_ELEMENT_WIDGET_H_
