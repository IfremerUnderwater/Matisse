#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "Processor.h"
#include "matlabcppinterface.h"


using namespace MatisseCommon;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class InitMatchModule : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

public:
    InitMatchModule();
    ~InitMatchModule();
    virtual void configure(Context * context, MatisseParameters * mosaicParameters);
    virtual void start();
    virtual void stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // InitMatchModule_H
