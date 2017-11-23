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
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "keep_unseen_faces");
}

Texturing3D::~Texturing3D(){

}

bool Texturing3D::configure()
{
    return true;
}

void Texturing3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

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
    Q_UNUSED(port)

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Convert model to mve
    QProcess featuresProc;
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start("openMVG_main_openMVG2MVE2 -i ./outReconstruction/sfm_data.bin -o ./outReconstruction/");

    while(featuresProc.waitForReadyRead(-1)){
        qDebug() << featuresProc.readAllStandardOutput();
    }

    // Texture model
    QString cmdLine = "texrecon";

    bool ok = false;
    bool keep_unseen_faces = _matisseParameters->getBoolParamValue("algo_param", "keep_unseen_faces", ok);
    if(ok && keep_unseen_faces)
        cmdLine += " --keep_unseen_faces";
    cmdLine += " ./outReconstruction/MVE::undistorted ";
    cmdLine += "./outReconstruction/" + fileNamePrefixStr + "_dense_mesh.ply ";
    cmdLine += "./outReconstruction/" + fileNamePrefixStr + "_texrecon";
    QProcess textureProc;
    textureProc.setWorkingDirectory(rootDirnameStr);
    textureProc.start(cmdLine);

    while(textureProc.waitForReadyRead(-1)){
        qDebug() << textureProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

