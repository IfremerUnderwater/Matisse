#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "Processor.h"

using namespace MatisseCommon;

/**
 * Init3DRecon
 * @brief  Module de début pour implementer un algorithme de traitement dans Matisse
 */
class Init3DRecon : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Init3DRecon")
#endif

public:
    Init3DRecon();
    ~Init3DRecon();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // InitMatchModule_H
