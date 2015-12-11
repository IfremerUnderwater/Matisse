#include "Init2DMosaic.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "GeoTransform.h"
#include "RasterGeoreferencer.h"

#include "MosaicDescriptor.h"
#include "MosaicDrawer.h"

#include "Polygon.h"

// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
Q_EXPORT_PLUGIN2(Init2DMosaic, Init2DMosaic)


Init2DMosaic::Init2DMosaic() :
    Processor(NULL, "Init2DMosaic", "Init 2D mosaic Descriptor with navigation", 1, 1)
{

    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param",  "V_Pose_C");
}

Init2DMosaic::~Init2DMosaic(){

}

bool Init2DMosaic::configure()
{
    return true;
}

void Init2DMosaic::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    qDebug() << logPrefix() << "Process InitMatchModule";

    // Forward image
    postImage(0, image);

}

bool Init2DMosaic::start()
{
    return true;
}

bool Init2DMosaic::stop()
{
    return true;
}

void Init2DMosaic::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;

    MosaicDescriptor *pMosaicD = new MosaicDescriptor;
    QVector<ProjectiveCamera*> *pCams = new QVector<ProjectiveCamera*>;
    ImageSet * imageSet;

    // Find imageSet corresponding to this port
    foreach (ImageSetPort *ImSet,*_inputPortList){
        if (ImSet->portNumber == port){
            imageSet = ImSet->imageSet;
            break;
        }
    }

    // Get camera matrix
    bool Ok;
    QMatrix3x3 qK = _matisseParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
    cv::Mat K(3,3,CV_64F);
    if (Ok){
        qDebug() << "K Ok =" << Ok;

        for (int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                K.at<qreal>(i,j) = qK(i,j);
            }
        }

    }else{
        qDebug() << "K Ok =" << Ok;
        exit(1);
    }

    std::cerr <<"K = " << K;

    double scaleFactor = _matisseParameters->getDoubleParamValue("algo_param", "scale_factor", Ok);

    if (!Ok){
        qDebug() << "scale factor not provided Ok \n";
        exit(1);
    }

    // Get camera lever arm
    cv::Mat V_T_C(3,1,CV_64F), V_R_C(3,3,CV_64F);

    Matrix6x1 V_Pose_C = _matisseParameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  Ok);
    if (Ok){

        for (int i=0; i<3; i++){
            V_T_C.at<qreal>(i,0) = V_Pose_C(0,i);
        }

        GeoTransform T;

        V_R_C = T.RotZ(V_Pose_C(0,5))*T.RotY(V_Pose_C(0,4))*T.RotX(V_Pose_C(0,3));

    }


    if (imageSet){

        // Create cameras
        QList<Image *> imageList = imageSet->getAllImages();
        foreach (Image* image, imageList) {

            NavImage *navImage = dynamic_cast<NavImage*>(image);
            if (navImage){
                pCams->push_back(new ProjectiveCamera(navImage , K, V_T_C, V_R_C, (qreal)scaleFactor));
            }else{
                qDebug() << "cannot cast as navImage \n";
                exit(1);
            }
        }

        // Init cameras
        pMosaicD->initCamerasAndFrames(*pCams,true);
        qDebug() << "Init done";

        // Compute extents
        pMosaicD->computeMosaicExtentAndShiftFrames();
        qDebug() << "Extent computation done";

        //Draw mosaic
        MosaicDrawer mosaicDrawer;
        cv::Mat mosaicImage,mosaicMask;
        mosaicDrawer.drawAndBlend(*pMosaicD, mosaicImage, mosaicMask);

        // Write geofile
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

        pMosaicD->writeToGeoTiff(mosaicImage,mosaicMask,outputDirnameStr + QDir::separator() + outputFilename);

    }else{
        qDebug()<<"No ImageSet acquired !";
        exit(1);
    }

    // Add pCams to mosaic _context
    QVariant * pCamsStocker = new QVariant();
    pCamsStocker->setValue(pCams);
    _context->addObject("Cameras", pCamsStocker);

    // Add pMosaicD to mosaic _context
    QVariant * pMosaicDStocker = new QVariant();
    pMosaicDStocker->setValue(pMosaicD);
    _context->addObject("MosaicDescriptor", pMosaicDStocker);

    // Flush next module port
    flush(0);

}

