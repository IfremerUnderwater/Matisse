#include "BridgeModule.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MosaicDescriptor.h"

#include "Polygon.h"

Q_EXPORT_PLUGIN2(BridgeModule, BridgeModule)


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

