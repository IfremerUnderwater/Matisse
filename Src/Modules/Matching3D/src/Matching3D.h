#ifndef InitMatchModule_H
#define InitMatchModule_H


#include "Processor.h"

using namespace MatisseCommon;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class Matching3D : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

public:
    Matching3D();
    ~Matching3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // InitMatchModule_H
