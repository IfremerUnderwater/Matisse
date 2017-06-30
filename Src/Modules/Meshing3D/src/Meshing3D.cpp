#include "Meshing3D.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


Q_EXPORT_PLUGIN2(Meshing3D, Meshing3D)



Meshing3D::Meshing3D() :
    Processor(NULL, "Meshing3D", "Create a mesh from 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");

}

Meshing3D::~Meshing3D(){

}

bool Meshing3D::configure()
{
    return true;
}

void Meshing3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);

}

bool Meshing3D::start()
{
    return true;
}

bool Meshing3D::stop()
{
    return true;
}

void Meshing3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    // densify
    QProcess denseProc;
    denseProc.setWorkingDirectory(rootDirnameStr);
    denseProc.start("openMVG_main_ConvertSfM_DataFormat -i ./outReconstruction/sfm_data.bin -o ./outReconstruction/sfm_data.json");

    while(denseProc.waitForReadyRead(-1)){
        qDebug() << denseProc.readAllStandardOutput();
    }

    //denseProc.start("InterfaceOpenMVG -i ./outReconstruction/sfm_data.json -o ./model.mvs");
    denseProc.start("openMVG_main_openMVG2openMVS -i ./outReconstruction/sfm_data.json -o ./model.mvs");

    while(denseProc.waitForReadyRead(-1)){
        qDebug() << denseProc.readAllStandardOutput();
    }

    denseProc.start("/usr/local/bin/OpenMVS/DensifyPointCloud ./model.mvs");

    while(denseProc.waitForReadyRead(-1)){
        qDebug() << denseProc.readAllStandardOutput();
    }


    // Compute Mesh
    QProcess meshProc;
    meshProc.setWorkingDirectory(rootDirnameStr);
    meshProc.start("/usr/local/bin/OpenMVS/ReconstructMesh ./model_dense.mvs");

    while(meshProc.waitForReadyRead(-1)){
        qDebug() << meshProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

