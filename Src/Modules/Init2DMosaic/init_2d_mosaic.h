#ifndef MATISSE_INIT_2D_MOSAIC_H_
#define MATISSE_INIT_2D_MOSAIC_H_


#include "processor.h"

namespace matisse {

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
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, Image &_image);
};

} // namespace matisse

#endif // MATISSE_INIT_2D_MOSAIC_H_

