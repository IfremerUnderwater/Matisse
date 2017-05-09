#include "SfmBundleAdjustment.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


Q_EXPORT_PLUGIN2(SfmBundleAdjustment, SfmBundleAdjustment)



SfmBundleAdjustment::SfmBundleAdjustment() :
    Processor(NULL, "SfmBundleAdjustment", "Estimate camera position and 3D sparse points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");

}

SfmBundleAdjustment::~SfmBundleAdjustment(){

}

bool SfmBundleAdjustment::configure()
{
    return true;
}

void SfmBundleAdjustment::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);

}

bool SfmBundleAdjustment::start()
{
    return true;
}

bool SfmBundleAdjustment::stop()
{
    return true;
}

void SfmBundleAdjustment::onFlush(quint32 port)
{
    Q_UNUSED(port)

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    // Compute Sfm bundle adjustment
    QProcess sfmProc;
    sfmProc.setWorkingDirectory(rootDirnameStr);
    sfmProc.start("openMVG_main_IncrementalSfM -i ./matches/sfm_data.json -m ./matches/ -o ./outReconstruction/");

    while(sfmProc.waitForReadyRead(-1)){
        qDebug() << sfmProc.readAllStandardOutput();
    }

    // Flush next module port
    flush(0);

}

