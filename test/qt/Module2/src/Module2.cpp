#include "Module2.h"
#include "MosaicContext.h"
#include "NavImage.h"
#include "FileImage.h"

// Exportation de la classe Module2 dans la bibliotheque de plugin TestModule2
Q_EXPORT_PLUGIN2(Module2, Module2)


Module2::Module2() :
    Processor(NULL, "Module2", "Module d'essai", 2, 1)
{

    qDebug()<< logPrefix() << " crée!";
}

bool Module2::configure()
{
    return true;
}

void Module2::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    qDebug() << logPrefix() << "Process Module2";


}

bool Module2::start()
{
    qDebug() << logPrefix() << "Enter start";
    QVariant *object = _context->getObject("MosaicData");
    if (object) {
        MosaicData * pm = object->value<MosaicData*>();
        qDebug()<< logPrefix() << "Receive this value: " << pm->init.filename;
    }
    qDebug() << logPrefix() << "Exit start";
    return true;
}

bool Module2::stop()
{
    qDebug() << logPrefix() << "Inside stop";
    return true;
}

void Module2::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;
    qDebug() << logPrefix() << "BLEND" ;


    foreach (ImageSetPort *imageSetPort, *_inputPortList ) {
        if (imageSetPort->portNumber == port) {
            ImageSet *imgSet = imageSetPort->imageSet;
            QList<Image*> images = imgSet->getAllImages();
            qDebug() << logPrefix() << "Processing all images" ;
            foreach (Image *image, images) {
                if (!isStarted())
                    break;
                NavImage* navImage = static_cast<NavImage*>(image);
                qDebug() << logPrefix() << "Chargement de l'image" ;
                navImage->imageData();
                Image* toDisplay = new Image(*navImage, false);
                emit signal_intermediateResult(toDisplay);
                navImage->releaseImageData();
            }
            break;
        }
    }

}



