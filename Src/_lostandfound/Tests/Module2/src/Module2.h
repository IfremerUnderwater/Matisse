#ifndef MODULE2_H
#define MODULE2_H

#include <QList>
#include <QVariant>
#include <QtDebug>

#include "processor.h"

using namespace MatisseCommon;

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class Module2 : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Module2")
#endif
public:
    Module2();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 port);
    virtual void onNewImage(quint32 port, Image &image);
};

#endif // MODULE2_H
