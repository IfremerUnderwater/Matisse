#include "Matching3D.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


Q_EXPORT_PLUGIN2(Matching3D, Matching3D)



Matching3D::Matching3D() :
    Processor(NULL, "Matching3D", "Match images and filter with geometric transformation", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");

}

Matching3D::~Matching3D(){

}

bool Matching3D::configure()
{
    return true;
}

void Matching3D::onNewImage(quint32 port, Image &image)
{

    // Forward image
    postImage(0, image);

}

bool Matching3D::start()
{
    return true;
}

bool Matching3D::stop()
{
    return true;
}

void Matching3D::onFlush(quint32 port)
{

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    // Compute Features
    QProcess featuresProc;
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start("openMVG_main_ComputeFeatures -i ./matches/sfm_data.json -o ./matches -p ULTRA");

    while(featuresProc.waitForReadyRead(-1)){
        qDebug() << featuresProc.readAllStandardOutput();
    }

    // Compute Matches
    QProcess matchingProc;
    matchingProc.setWorkingDirectory(rootDirnameStr);
    matchingProc.start("openMVG_main_ComputeMatches -i ./matches/sfm_data.json -o ./matches");

    while(matchingProc.waitForReadyRead(-1)){
        qDebug() << matchingProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

