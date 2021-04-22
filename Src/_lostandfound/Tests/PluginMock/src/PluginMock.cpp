#include "PluginMock.h"
#include <QLibrary>

Q_EXPORT_PLUGIN2(PluginMock, PluginMockImpl)


PluginMockImpl::PluginMockImpl() :
        Processor(NULL, "PluginMockImpl", "Module de test de DLL", 1, 1)
{
    qDebug() << "Plugin ctr." << endl;
}

PluginMockImpl::~PluginMockImpl() {
    myLib->unload();
}

void PluginMockImpl::configure(Context *context, MatisseParameters *mosaicParameters)
{
    qDebug() << "Plugin configure." << endl;
    // Chargement de la DLL
    QString libName = "modules/AlgoMock.dll";
    myLib = new QLibrary(libName);

    myLib->load();

    initFunction = (DLL_FUNCTION) myLib->resolve("init");
    runFunction = (DLL_FUNCTION) myLib->resolve("doWork");
    // appel du init de la DLL
    if (initFunction)
        initFunction();
}


void PluginMockImpl::stop()
{
    qDebug() << "Plugin stop." << endl;

}

void PluginMockImpl::onNewImage(quint32 port, Image &image)
{
    qDebug() << "Plugin onNewImage." << endl;
}


void PluginMockImpl::start()
{
    qDebug() << "Plugin start." << endl;

    // Use Mosaic Parameters Here
    //_mosaicParameters->getBoolParamValue();
    // appel du run de la DLL
    if (runFunction)
        runFunction();

}

