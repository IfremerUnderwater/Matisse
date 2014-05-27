#ifndef PLUGINMOCK_H
#define PLUGINMOCK_H

#include <QList>
#include <QVariant>
#include <QtDebug>

#include "Processor.h"

using namespace MatisseCommon;

typedef void (*DLL_FUNCTION)(void);

class PluginMockImpl : public Processor
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::Processor)

public:
    PluginMockImpl();
    virtual ~PluginMockImpl();

    virtual void configure(Context * context, MatisseParameters * mosaicParameters);

    ///
    /// \brief La méthode start est appelée pour prevenir du commencement de l'execution d'un asssemblage.
    ///
    virtual void start();

    ///
    /// \brief La méthode stop est appelée pour prevenir de la fin de l'execution d'un asssemblage.
    ///
    virtual void stop();


    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param port
    /// \param image
    ///
    virtual void onNewImage(quint32 port, Image &image);
private:
    DLL_FUNCTION initFunction;
    DLL_FUNCTION runFunction;
    QLibrary *myLib;
};

#endif // PLUGINMOCK_H
