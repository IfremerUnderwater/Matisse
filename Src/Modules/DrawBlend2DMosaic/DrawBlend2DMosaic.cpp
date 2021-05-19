#if _MSC_VER
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "file_utils.h"
//#include "MosaicContext.h"
#include "mosaic_drawer.h"
#include "NavImage.h"
#include "DrawBlend2DMosaic.h"
#include "GeoTransform.h"
#include "RasterGeoreferencer.h"
#include "mosaic_descriptor.h"

using namespace cv;

// Export de la classe DrawAndWriteModule dans la bibliotheque de plugin DrawAndWriteModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(DrawBlend2DMosaic, DrawBlend2DMosaic)
#endif


DrawBlend2DMosaic::DrawBlend2DMosaic() :
    OutputDataWriter(NULL, "DrawBlend2DMosaic", "DrawBlend2DMosaic", 1)
{
    qDebug() << logPrefix() << "DrawBlend2DMosaic in constructor...";

    addExpectedParameter("dataset_param",  "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param",  "output_filename");

    addExpectedParameter("algo_param", "block_drawing");
    addExpectedParameter("algo_param", "block_width");
    addExpectedParameter("algo_param", "block_height");

    addExpectedParameter("algo_param", "disjoint_drawing");
    addExpectedParameter("algo_param", "single_image_output");

}

DrawBlend2DMosaic::~DrawBlend2DMosaic(){

}

bool DrawBlend2DMosaic::configure()
{
    qDebug() << logPrefix() << "configure";

    QString datasetDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    _outputDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    /* Resolve UNIX paths ('~/...') for remote job execution */
    _outputDirnameStr = FileUtils::resolveUnixPath(_outputDirnameStr);

    QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    if (datasetDirnameStr.isEmpty()
     || _outputDirnameStr.isEmpty()
     || outputFilename.isEmpty())
        return false;


    QFileInfo outputDirInfo(_outputDirnameStr);
    QFileInfo datasetDirInfo(datasetDirnameStr);

    bool isRelativeDir = outputDirInfo.isRelative();

    if (isRelativeDir) {
        _outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + _outputDirnameStr);
    }
    _rastersInfo.clear();

    return true;
}

void DrawBlend2DMosaic::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port);
    Q_UNUSED(image);
}

QList<QFileInfo> DrawBlend2DMosaic::rastersInfo()
{
    return _rastersInfo;
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

    // Get drawing prefix
    QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    emit signal_processCompletion(10);

    //Draw mosaic
    MosaicDrawer mosaicDrawer;
    QFileInfo outputFileInfo;

    QString output_type_choice = _matisseParameters->getStringParamValue("algo_param", "single_image_output");

    bool draw_geotiff = true;
    bool draw_jpeg = false;
    if (output_type_choice == "Geotiff only")
    {
        draw_geotiff = true;
        draw_jpeg = false;
    }
    if (output_type_choice == "JPEG only")
    {
        draw_geotiff = false;
        draw_jpeg = true;
    }
    if (output_type_choice == "Geotiff and JPEG")
    {
        draw_geotiff = true;
        draw_jpeg = true;
    }


    if (_matisseParameters->getBoolParamValue("algo_param", "disjoint_drawing", Ok)) {
        QStringList outputFiles;
        
        if (draw_geotiff)
        {
            outputFiles = mosaicDrawer.writeImagesAsGeoTiff(*pMosaicD,
                _outputDirnameStr, outputFilename);
        }

        if (draw_jpeg)
        {
            mosaicDrawer.outputMosaicImagesAsIs(*pMosaicD,
                _outputDirnameStr, outputFilename);
        }

        foreach(QString filename, outputFiles) {
            outputFileInfo.setFile(QDir(_outputDirnameStr), filename);
            _rastersInfo << outputFileInfo;
        }

    }
    else if (!blockDraw){
        // opencv331
        cv::UMat mosaicImage,mosaicMask;
        mosaicDrawer.drawAndBlend(*pMosaicD, mosaicImage, mosaicMask);

        emit signal_processCompletion(50);

        // copy mask to force data pointer allocation in the right order
        Mat maskCopy;
        mosaicMask.copyTo(maskCopy);
        mosaicMask.release();
        Mat matMosaicImage = mosaicImage.getMat(ACCESS_READ);
        // Write geofile
        pMosaicD->writeToGeoTiff(matMosaicImage,maskCopy,_outputDirnameStr + QDir::separator() + outputFilename + ".tiff");

        outputFileInfo.setFile(QDir(_outputDirnameStr), outputFilename+ ".tiff");
        _rastersInfo << outputFileInfo;

    }else{
        qDebug()<< logPrefix() << "entered block drawing part...";
       
        
        QStringList outputFiles = mosaicDrawer.blockDrawBlendAndWrite(*pMosaicD,
                                                                      Point2d(blockWidth, blockHeight),
                                                                      _outputDirnameStr, outputFilename);
        foreach (QString filename, outputFiles) {
            outputFileInfo.setFile(QDir(_outputDirnameStr), filename);
            _rastersInfo << outputFileInfo;
        }

    }

    emit signal_processCompletion(100);
}



