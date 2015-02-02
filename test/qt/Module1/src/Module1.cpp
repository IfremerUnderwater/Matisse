#include "Module1.h"
#include "MosaicContext.h"
#include "NavImage.h"
#include "GeoTransform.h"

// Exportation de la classe Module1 dans la bibliotheque de plugin TestModule1
Q_EXPORT_PLUGIN2(Module1, Module1)


Module1::Module1() :
    Processor(NULL, "Module1", "Module d'essai", 1, 2)
{
    addExpectedParameter("algo_param", "sift_code");
}

bool Module1::configure()
{
    return true;
}

void Module1::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    QString sift_code = _matisseParameters->getStringParamValue("algo_param", "sift_code");
    qDebug() << logPrefix() << "sift_code: " << sift_code;
    quint8 count = 0;
    while (count < 5) {
        qDebug() << logPrefix() << "COUNT=" << count;
        count++;
    }

    GeoTransform TGeo;
    NavImage * ImageNav = (NavImage*)(&image);
    qDebug()<< ImageNav->navInfo().dump();

    qreal x,y,lat,lon;
    QString utmZone;
    TGeo.LatLongToUTM(ImageNav->navInfo().latitude(),ImageNav->navInfo().longitude(),x,y,utmZone);
    qDebug()<< "X = " << x << " Y = " << y << " UTMzone = " << utmZone;

    TGeo.UTMToLatLong(x,y,utmZone,lat,lon);
    qDebug()<< "lat = " << lat << " lon = " << lon ;

    // Forward image
    postImage(1, image);
}

bool Module1::start()
{
    qDebug() << logPrefix() << "Enter start";

    MosaicData * pm = new MosaicData();
    QVariant * v = new QVariant();

    pm->init.filename = QString("Test\n");
    v->setValue(pm);

    _context->addObject("MosaicData", v);
    qDebug() << logPrefix() << "Exit start";
    return true;
}



bool Module1::stop()
{
    qDebug() << logPrefix() << "Inside stop";
    return true;
}


void Module1::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;
    qDebug() << logPrefix() << "OPTIM" ;
    flush(1);
}




