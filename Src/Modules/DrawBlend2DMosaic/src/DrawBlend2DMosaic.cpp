#include "MosaicContext.h"
#include "MosaicDrawer.h"
#include "NavImage.h"
#include "DrawBlend2DMosaic.h"
#include "GeoTransform.h"
#include "RasterGeoreferencer.h"
#include "MosaicDescriptor.h"

// Export de la classe DrawAndWriteModule dans la bibliotheque de plugin DrawAndWriteModule
Q_EXPORT_PLUGIN2(DrawBlend2DMosaic, DrawBlend2DMosaic)



DrawBlend2DMosaic::DrawBlend2DMosaic() :
    Processor(NULL, "DrawBlend2DMosaic", "DrawBlend2DMosaic", 1, 1)
{
    qDebug() << logPrefix() << "DrawBlend2DMosaic in constructor...";

    addExpectedParameter("dataset_param",  "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param",  "output_filename");

    addExpectedParameter("algo_param", "block_drawing");
    addExpectedParameter("algo_param", "block_width");
    addExpectedParameter("algo_param", "block_height");

}

DrawBlend2DMosaic::~DrawBlend2DMosaic(){

}

bool DrawBlend2DMosaic::configure()
{
    return true;
}

void DrawBlend2DMosaic::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port);
    Q_UNUSED(image);
}

bool DrawBlend2DMosaic::start()
{
    return true;
}

bool DrawBlend2DMosaic::stop()
{
    return true;
}

void DrawBlend2DMosaic::onFlush(quint32 port)
{

    emit signal_processCompletion(0);
    emit signal_userInformation("Drawing and blending 2D mosaic...");

    MosaicDescriptor *pMosaicD = NULL;
    //QVector<ProjectiveCamera*> *pCams = NULL;

    // Get pCams from mosaic _context
    /*QVariant * pCamsStocker = _context->getObject("Cameras");
    if (pCamsStocker) {
        pCams = pCamsStocker->value< QVector<ProjectiveCamera*>* >();
        qDebug()<< logPrefix() << "Receiving Cameras on port : " << port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << port;
    }*/

    // Get pMosaicD from mosaic _context
    QVariant * pMosaicDStocker = _context->getObject("MosaicDescriptor");
    if (pMosaicDStocker) {
        pMosaicD = pMosaicDStocker->value<MosaicDescriptor *>();
        qDebug()<< logPrefix() << "Receiving MosaicDescriptor on port : " << port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << port;
    }

    // Get drawing parameters
    bool Ok;
    bool blockDraw = _matisseParameters->getBoolParamValue("algo_param", "block_drawing", Ok);
    qDebug() << logPrefix() << "block_drawing = " << blockDraw;

    int blockWidth = _matisseParameters->getIntParamValue("algo_param", "block_width", Ok);
    qDebug() << logPrefix() << "block_width = " << blockWidth;

    int blockHeight = _matisseParameters->getIntParamValue("algo_param", "block_height", Ok);
    qDebug() << logPrefix() << "block_height = " << blockHeight;

    // Get drawing path
    QString datasetDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    QString outputDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    if (outputDirnameStr.isEmpty()
            || datasetDirnameStr.isEmpty()
            || outputFilename.isEmpty())
        return;

    QFileInfo outputDirInfo(outputDirnameStr);
    QFileInfo datasetDirInfo(datasetDirnameStr);

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
    }

    qDebug() << "output_dir = " << outputDirnameStr;
    qDebug() << "output_filename = " << outputFilename;

    emit signal_processCompletion(10);

    //Draw mosaic
    MosaicDrawer mosaicDrawer;    

    if (!blockDraw){

        cv::Mat mosaicImage,mosaicMask;
        mosaicDrawer.drawAndBlend(*pMosaicD, mosaicImage, mosaicMask);

        emit signal_processCompletion(50);

        // Write geofile
        pMosaicD->writeToGeoTiff(mosaicImage,mosaicMask,outputDirnameStr + QDir::separator() + outputFilename);

    }else{
        qDebug()<< logPrefix() << "entered block drawing part...";

        mosaicDrawer.blockDrawBlendAndWrite(*pMosaicD,
                               Point2d(blockWidth, blockHeight),
                               outputDirnameStr + QDir::separator() + QString("MosaicOut"));

    }

    emit signal_processCompletion(100);
}



