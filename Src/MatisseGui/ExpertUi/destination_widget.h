#ifndef MATISSE_DESTINATION_WIDGET_H_
#define MATISSE_DESTINATION_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class DestinationWidget : public ElementWidget
{
//    Q_OBJECT
public:

    virtual void drawSymbol(QPainter *_painter, bool _for_icon = false);
    virtual int type() const { return DESTINATION_TYPE; }

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_DESTINATION_WIDGET_H_
