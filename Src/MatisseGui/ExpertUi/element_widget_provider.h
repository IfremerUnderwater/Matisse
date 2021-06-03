#ifndef MATISSE_ELEMENT_WIDGET_PROVIDER_H_
#define MATISSE_ELEMENT_WIDGET_PROVIDER_H_

#include "source_widget.h"
#include "processor_widget.h"
#include "destination_widget.h"


namespace matisse {

class ElementWidgetProvider
{
public:
    virtual SourceWidget * getSourceWidget(QString name) = 0;
    virtual ProcessorWidget * getProcessorWidget(QString name) = 0;
    virtual DestinationWidget * getDestinationWidget(QString name) = 0;
};

} // namespace matisse

#endif // MATISSE_ELEMENT_WIDGET_PROVIDER_H_
