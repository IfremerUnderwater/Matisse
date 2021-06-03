#ifndef MATISSE_PIPE_WIDGET_H_
#define MATISSE_PIPE_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class PipeWidget : public ElementWidget
{

public:
    PipeWidget(QGraphicsItem *parent = 0);

    virtual void drawSymbol(QPainter *painter, bool forIcon = false);
    void setGeometry(int x, int y, int deltaYt = 0, int deltaXt = 0, int deltaY = 0, int deltaXb = 0, int deltaYb = 0, QColor color = QColor());
    void setColor(QColor color);
    void fromSourcePos(QPointF srcPos);
    void toDestinationPos(QPointF dstPos);
    virtual int type() const { return PipeType;}

    void setStart(bool temp, ElementWidget * src, int srcLine);
    void setEnd(ElementWidget *dest, int destLine);
    ElementWidget * getStartElement();
    ElementWidget *getTempStartElement();
    ElementWidget * getEndElement();
    int getStartElementLine();
    int getEndElementLine();
    void refreshConnections();

    virtual void clone(PipeWidget * other);

    static void setCurrentColor(QRgb color);
    static QRgb getNextColor();
    bool hasStart();
    void clear();

private:
    static int _currentColorIndex;
    static QList<QRgb> _colorsList;
    static void fillColorsList();

    int _x;
    int _y;
    int _deltaYt;
    int _deltaXt;
    int _deltaY;
    int _deltaXb;
    int _deltaYb;

    ElementWidget * _startElt;
    ElementWidget * _tempStartElt;
    ElementWidget * _endElt;
    int _startLine;
    int _endLine;

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_PIPE_WIDGET_H_
