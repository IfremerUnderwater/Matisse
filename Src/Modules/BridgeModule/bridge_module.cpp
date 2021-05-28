#include "bridge_module.h"
#include "mosaic_descriptor.h"
#include "nav_image.h"
#include "Polygon.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(BridgeModule, BridgeModule)
#endif

BridgeModule::BridgeModule() :
    Processor(NULL, "BridgeModule", "This bridge does nothing for the moment", 1, 1)
{

}

BridgeModule::~BridgeModule(){

}

bool BridgeModule::configure()
{
    return true;
}

void BridgeModule::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;

    // Forward image
    postImage(0, image);

}

bool BridgeModule::start()
{
    setOkStatus();
    return true;
}

bool BridgeModule::stop()
{
    return true;
}

void BridgeModule::onFlush(quint32 port)
{
    Q_UNUSED(port)
    // Flush next module port
    flush(0);

}

