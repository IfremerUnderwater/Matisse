#ifndef PROCESSORWIDGET_H
#define PROCESSORWIDGET_H

#include "ElementWidget.h"

namespace MatisseServer {
class ProcessorWidget : public ElementWidget
{
//    Q_OBJECT

public:
    virtual void drawSymbol(QPainter *painter, bool forIcon = false);
    virtual int type() const { return ProcessorType;}


signals:
    
public slots:
    
};
}

#endif // PROCESSORWIDGET_H
