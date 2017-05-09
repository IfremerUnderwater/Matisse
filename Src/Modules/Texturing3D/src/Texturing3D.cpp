#include "Texturing3D.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


Q_EXPORT_PLUGIN2(Texturing3D, Texturing3D)



Texturing3D::Texturing3D() :
    Processor(NULL, "Texturing3D", "T 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");

}

Texturing3D::~Texturing3D(){

}

bool Texturing3D::configure()
{
    return true;
}

void Texturing3D::onNewImage(quint32 port, Image &image)
{

    // Forward image
    postImage(0, image);

}

bool Texturing3D::start()
{
    return true;
}

bool Texturing3D::stop()
{
    return true;
}

void Texturing3D::onFlush(quint32 port)
{

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    // Convert model to mve
    QProcess featuresProc;
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start("openMVG_main_openMVG2MVE2 -i ./outReconstruction/sfm_data.bin -o ./outReconstruction/");

    while(featuresProc.waitForReadyRead(-1)){
        qDebug() << featuresProc.readAllStandardOutput();
    }

    // Texture model
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start("texrecon ./outReconstruction/MVE::undistorted ./model_dense_mesh.ply model_texrecon");

    while(featuresProc.waitForReadyRead(-1)){
        qDebug() << featuresProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

