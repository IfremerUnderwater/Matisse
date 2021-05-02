#include "Init2DMosaic.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MosaicDescriptor.h"

#include "Polygon.h"
#include <QMessageBox>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init2DMosaic, Init2DMosaic)
#endif

Init2DMosaic::Init2DMosaic() :
    Processor(NULL, "Init2DMosaic", "Init 2D mosaic Descriptor with navigation", 1, 1)
{

    //addExpectedParameter("cam_param",  "K");
    //addExpectedParameter("algo_param", "scale_factor");
    //addExpectedParameter("cam_param",  "V_Pose_C");
    addExpectedParameter("cam_param", "camera_equipment");

    addExpectedParameter("algo_param","filter_overlap");
    addExpectedParameter("algo_param","min_overlap");
    addExpectedParameter("algo_param","max_overlap");
    addExpectedParameter("algo_param", "disjoint_drawing");

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
    setOkStatus();

    return true;
}

bool Init2DMosaic::stop()
{
    return true;
}

void Init2DMosaic::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;

    emit signal_processCompletion(0);
    emit signal_userInformation("Initializing 2D Mosaic...");

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

    int images_width;

    if (imageSet)
    {
        QList<Image*> imageList = imageSet->getAllImages();
        images_width = imageList[0]->width();
    }
    else
    {
        emit signal_showErrorMessage(tr("Did not find any image"), tr("Did not find any image."));
        return;
    }

    emit signal_processCompletion(10);

    bool Ok = false;

    // Get camera equipment
    CameraInfo camera_equipment = _matisseParameters->getCamInfoParamValue("cam_param", "camera_equipment", Ok);

    if (!Ok)
    {
        emit signal_showErrorMessage(tr("Did not find the camera equipment"), tr("Did not found the camera equipment. Please check that the required equipment is available in the database."));
        return;
    }


    // Fill camera matrix
    int full_sensor_width, full_sensor_height;
    camera_equipment.fullSensorSize(full_sensor_width, full_sensor_height);

    cv::Mat K = camera_equipment.K();

    double scaling_factor = images_width / (double)full_sensor_width;
    K.at<double>(0, 0) = scaling_factor * K.at<double>(0, 0);
    K.at<double>(1, 1) = scaling_factor * K.at<double>(1, 1);
    K.at<double>(0, 2) = scaling_factor * K.at<double>(0, 2);
    K.at<double>(1, 2) = scaling_factor * K.at<double>(1, 2);

    std::cout << "K = " << K << std::endl;

    emit signal_processCompletion(30);


    // Get camera lever arm (inverted signs are due to different frames conventions between mosaicking and Matisse)
    cv::Mat V_T_C(3,1,CV_64F), V_R_C(3,3,CV_64F);
    cv::Mat vehicle_to_cam_trans = camera_equipment.vehicleToCameraTransform();

    V_T_C.at<double>(0,0) = vehicle_to_cam_trans.at<double>(0,0);
    V_T_C.at<double>(1,0) = -vehicle_to_cam_trans.at<double>(0, 1);
    V_T_C.at<double>(2,0) = -vehicle_to_cam_trans.at<double>(0, 2);

    GeoTransform T;

    V_R_C = T.RotZ(-vehicle_to_cam_trans.at<double>(0, 5))*T.RotY(-vehicle_to_cam_trans.at<double>(0, 4))*T.RotX(vehicle_to_cam_trans.at<double>(0, 3));

    std::cout << "V_T_C = " << V_T_C << std::endl;
    std::cout << "V_R_C = " << V_R_C << std::endl;
    std::cout << "vehicle_to_cam_trans = " << vehicle_to_cam_trans << std::endl;

    emit signal_processCompletion(60);

    if (imageSet){

        // Create cameras
        QList<Image *> imageList = imageSet->getAllImages();
        foreach (Image* image, imageList) {

            NavImage *navImage = dynamic_cast<NavImage*>(image);
            if (navImage) {
                if (navImage->navInfo().altitude()>0.0)
                {
                    pCams->push_back(new ProjectiveCamera(navImage, K, V_T_C, V_R_C, 1.0));
                }
            }else{
                qDebug() << "cannot cast as navImage \n";
                exit(1);
            }
        }

        // Init cameras
        pMosaicD->initCamerasAndFrames(*pCams,true);
        qDebug() << "Init done";

        // Filter cameras on overlap
        if (_matisseParameters->getBoolParamValue("algo_param", "disjoint_drawing", Ok)) {
            pMosaicD->computeMosaicExtentAndShiftFrames();
            pMosaicD->decimateImagesUntilNoOverlap();
        }
        else if ( _matisseParameters->getBoolParamValue("algo_param","filter_overlap", Ok) ){
            double min_overlap = _matisseParameters->getDoubleParamValue("algo_param","min_overlap", Ok);
            double max_overlap = _matisseParameters->getDoubleParamValue("algo_param","max_overlap", Ok);

            pMosaicD->computeMosaicExtentAndShiftFrames();
            pMosaicD->decimateImagesFromOverlap(min_overlap, max_overlap);
        }

        // Compute extents
        pMosaicD->computeMosaicExtentAndShiftFrames();
        qDebug() << "Extent computation done";

    }

    emit signal_processCompletion(90);

    // Add pCams to mosaic _context
    QVariant * pCamsStocker = new QVariant();
    pCamsStocker->setValue(pCams);
    _context->addObject("Cameras", pCamsStocker);

    // Add pMosaicD to mosaic _context
    QVariant * pMosaicDStocker = new QVariant();
    pMosaicDStocker->setValue(pMosaicD);
    _context->addObject("MosaicDescriptor", pMosaicDStocker);

    emit signal_processCompletion(100);


    // Flush next module port
    flush(0);

}

