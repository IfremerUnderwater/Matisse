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
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "resolution_level");
    addExpectedParameter("algo_param", "number_views_fuse");
    addExpectedParameter("algo_param", "decimate_factor");
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

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // densify
//    QProcess denseProc;
//    denseProc.setWorkingDirectory(rootDirnameStr);
//    denseProc.start("openMVG_main_ConvertSfM_DataFormat -i ./outReconstruction/sfm_data.bin -o ./outReconstruction/sfm_data.json");

//    while(denseProc.waitForReadyRead(-1)){
//        qDebug() << denseProc.readAllStandardOutput();
//    }

    //denseProc.start("InterfaceOpenMVG -i ./outReconstruction/sfm_data.json -o ./model.mvs");
    //denseProc.start("openMVG_main_openMVG2openMVS -i ./outReconstruction/sfm_data.json -o ./model.mvs");

    QString cmdline = "openMVG_main_openMVG2openMVS -i ./outReconstruction/sfm_data.bin -o ";
    cmdline += "./outReconstruction/" + fileNamePrefixStr + ".mvs";
    QProcess cvtProc;
    cvtProc.setWorkingDirectory(rootDirnameStr);
    cvtProc.start(cmdline);

    while(cvtProc.waitForReadyRead(-1)){
        qDebug() << cvtProc.readAllStandardOutput();
    }

    // densify
    cmdline = "/usr/local/bin/OpenMVS/DensifyPointCloud";
    bool ok = true;
    int reslevel = _matisseParameters->getIntParamValue("algo_param", "resolution_level", ok);
    if(ok)
        cmdline += " --resolution-level " + QString::number(reslevel);
    int nbviewfuse =  _matisseParameters->getIntParamValue("algo_param", "number_views_fuse", ok);
    if(ok)
        cmdline += " --number-views-fuse " + QString::number(nbviewfuse);

    cmdline += " ./outReconstruction/" + fileNamePrefixStr + ".mvs";
    QProcess denseProc;
    denseProc.setWorkingDirectory(rootDirnameStr);
    denseProc.start(cmdline);

    while(denseProc.waitForReadyRead(-1)){
        qDebug() << denseProc.readAllStandardOutput();
    }


    // Compute Mesh
    cmdline = "/usr/local/bin/OpenMVS/ReconstructMesh";
    double decimatearg = _matisseParameters->getDoubleParamValue("algo_param", "decimate_factor", ok);
    if(ok)
        cmdline += " --decimate " + QString::number(decimatearg);

    cmdline +=  " ./outReconstruction/" + fileNamePrefixStr + "_dense.mvs";
    QProcess meshProc;
    meshProc.setWorkingDirectory(rootDirnameStr);
    meshProc.start(cmdline);

    while(meshProc.waitForReadyRead(-1)){
        qDebug() << meshProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

