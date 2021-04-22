#include "ModuleImplementation.h"

Q_EXPORT_PLUGIN2(TestBibliotheque, ModuleImplementation)


ModuleImplementation::ModuleImplementation()
{
    init();
}

/*
ModuleImplementation * ModuleImplementation::newInstance()
{
    ModuleImplementation * newInstance = new ModuleImplementation;
    newInstance->init();
    return newInstance;
}
*/


void ModuleImplementation::run()
{
    qDebug() << "Lecture param algo_param/WriteGeoTiff" << _mosaicParameters->getStringParamValue("algo_param", "WriteGeoTiff");
    qDebug() << "Remplissage mosaic";

    qDebug() << "Process";
    quint8 count = 0;
    while (count < 10) {
        qDebug() << "COUNT=" << count;
        msleep(1800);
        count++;
    }
}

void ModuleImplementation::init()
{
    // definition du processorId généré

    // definition du processorName
    _processorName = "Processeur de test 2";
}
