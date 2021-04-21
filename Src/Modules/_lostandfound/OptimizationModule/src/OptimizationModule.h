#ifndef OptimizationModule_H
#define OptimizationModule_H


#include "Processor.h"
#include "matlabcppinterface.h"


using namespace MatisseCommon;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class OptimizationModule : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

public:
    OptimizationModule();
    ~OptimizationModule();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // OptimizationModule_H
