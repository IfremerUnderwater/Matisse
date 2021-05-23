#ifndef DESTINATIONWIDGET_H
#define DESTINATIONWIDGET_H

#include "ElementWidget.h"

namespace MatisseServer {
class DestinationWidget : public ElementWidget
{
//    Q_OBJECT
public:

    virtual void drawSymbol(QPainter *painter, bool forIcon = false);
    virtual int type() const { return DestinationType; }

signals:
    
public slots:
    
};
}

#endif // DESTINATIONWIDGET_H
