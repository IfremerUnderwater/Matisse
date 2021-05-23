#ifndef ELEMENTWIDGETPROVIDER_H
#define ELEMENTWIDGETPROVIDER_H

#include "source_widget.h"
#include "processor_widget.h"
#include "destination_widget.h"


namespace MatisseServer {

class ElementWidgetProvider
{
public:
    virtual SourceWidget * getSourceWidget(QString name) = 0;
    virtual ProcessorWidget * getProcessorWidget(QString name) = 0;
    virtual DestinationWidget * getDestinationWidget(QString name) = 0;
};

}

#endif // ELEMENTWIDGETPROVIDER_H
