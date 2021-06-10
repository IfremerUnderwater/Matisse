#ifndef MATISSE_BRIDGE_MODULE_H_
#define MATISSE_BRIDGE_MODULE_H_


#include "processor.h"

namespace matisse {

/**
 * BridgeModule
 * @brief  This module does nothing
 */
class BridgeModule : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "BridgeModule")
#endif

public:
    BridgeModule();
    ~BridgeModule();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, Image &_image);
};

} // namespace matisse

#endif // MATISSE_BRIDGE_MODULE_H_
