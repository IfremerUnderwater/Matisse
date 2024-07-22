#pragma once

#include "processor.h"

namespace matisse {

/**
 * InterfaceColmap
 * @brief  Module de début pour implementer un algorithme de traitement dans Matisse
 */
class InterfaceColmap : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "InterfaceColmap")
#endif

private:


public:
    InterfaceColmap();
    ~InterfaceColmap();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    bool Initialize();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, Image &_image);
};

} // namespace matisse

