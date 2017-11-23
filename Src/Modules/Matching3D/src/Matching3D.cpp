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
    addExpectedParameter("algo_param", "force_recompute");
    addExpectedParameter("algo_param", "describer_method");
    addExpectedParameter("algo_param", "describer_preset");
    addExpectedParameter("algo_param", "nearest_matching_method");
    addExpectedParameter("algo_param", "video_mode_matching");
}

Matching3D::~Matching3D(){

}

bool Matching3D::configure()
{
    return true;
}

void Matching3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)
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
    Q_UNUSED(port)

    emit signal_processCompletion(0);
    emit signal_userInformation("Matching 3D : Compute Features...");

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    bool ok = true;
    bool force_recompute = _matisseParameters->getBoolParamValue("algo_param", "force_recompute", ok);

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();
    QString threadsParam = " -n ";
    threadsParam += QString::number(nbthreads);

    // describer preset parameter
    QString presetParam = " -p ";
    QString presetParamval = _matisseParameters->getStringParamValue("algo_param", "describer_preset");
    presetParam += presetParamval;

    QString commandLine = "openMVG_main_ComputeFeatures -i ./matches/sfm_data.json -o ./matches";
    if(force_recompute)
         commandLine += " -f 1";
    commandLine += threadsParam;
    commandLine += presetParam;

    // Compute Features
    QProcess featuresProc;
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start(commandLine);

    while(featuresProc.waitForReadyRead(-1)){
        qDebug() << featuresProc.readAllStandardOutput();
    }

    emit signal_processCompletion(50);
    emit signal_userInformation("Matching 3D : Compute Matches...");

    // Compute Matches
    commandLine = "openMVG_main_ComputeMatches -i ./matches/sfm_data.json -o ./matches";
    if(force_recompute)
         commandLine += " -f 1";
    // nearest matching method
    QString nmmParam = " -n ";
    QString nmmParamValue = _matisseParameters->getStringParamValue("algo_param", "nearest_matching_method");
    nmmParam += nmmParamValue;

    commandLine += nmmParam;

    // video mode matching
    QString vmmParam = " -v ";
    ok = true;
    int vmmParamVal = _matisseParameters->getIntParamValue("algo_param", "video_mode_matching", ok);
    vmmParam +=  QString::number(vmmParamVal);
    if(ok)
        commandLine += vmmParam;

    QProcess matchingProc;
    matchingProc.setWorkingDirectory(rootDirnameStr);
    matchingProc.start(commandLine);

    while(matchingProc.waitForReadyRead(-1)){
        qDebug() << matchingProc.readAllStandardOutput();
    }

    emit signal_processCompletion(100);
    emit signal_userInformation("Matching 3D - ended");

    // Flush next module port
    flush(0);

}

