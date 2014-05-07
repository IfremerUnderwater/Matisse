#ifndef PARAMETERSWIDGET_H
#define PARAMETERSWIDGET_H

#include "ElementWidget.h"

namespace MatisseServer {
class ParametersWidget : public ElementWidget
{
//    Q_OBJECT
public:

    virtual void drawSymbol(QPainter *painter, bool forIcon = false);

    virtual int type() const { return ParametersType;}
signals:
    
public slots:
    
};
}

#endif // PARAMETERSWIDGET_H
