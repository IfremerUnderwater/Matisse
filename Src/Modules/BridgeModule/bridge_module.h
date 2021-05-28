#ifndef BridgeModule_H
#define BridgeModule_H


#include "processor.h"

using namespace MatisseCommon;

/**
 * BridgeModule
 * @brief  This module does nothing
 */
class BridgeModule : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "BridgeModule")
#endif

public:
    BridgeModule();
    ~BridgeModule();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // BridgeModule_H
