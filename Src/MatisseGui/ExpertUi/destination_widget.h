#ifndef MATISSE_DESTINATION_WIDGET_H_
#define MATISSE_DESTINATION_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class DestinationWidget : public ElementWidget
{
//    Q_OBJECT
public:

    virtual void drawSymbol(QPainter *painter, bool forIcon = false);
    virtual int type() const { return DestinationType; }

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_DESTINATION_WIDGET_H_
