#ifndef MATISSE_PROCESSOR_WIDGET_H_
#define MATISSE_PROCESSOR_WIDGET_H_

#include "element_widget.h"

namespace matisse {

class ProcessorWidget : public ElementWidget
{
//    Q_OBJECT

public:
    virtual void drawSymbol(QPainter *_painter, bool _for_icon = false);
    virtual int type() const { return PROCESSOR_TYPE;}


signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_PROCESSOR_WIDGET_H_
