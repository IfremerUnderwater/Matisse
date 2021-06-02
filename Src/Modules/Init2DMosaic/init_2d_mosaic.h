#ifndef Init2DMosaicModule_H
#define Init2DMosaicModule_H


#include "processor.h"

using namespace matisse;

/**
 * Init2DMosaic
 * @brief
 */
class Init2DMosaic : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Init2DMosaic")
#endif

public:
    Init2DMosaic();
    ~Init2DMosaic();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // Init2DMosaicModule_H

