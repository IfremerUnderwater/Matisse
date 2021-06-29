#include "init_2d_mosaic.h"
#include "nav_image.h"

#include "mosaic_descriptor.h"

#include "Polygon.h"
#include <QMessageBox>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init2DMosaic, Init2DMosaic)
#endif

using namespace nav_tools;
using namespace optical_mapping;

namespace matisse {

Init2DMosaic::Init2DMosaic() :
    Processor(NULL, "Init2DMosaic", "Init 2D mosaic Descriptor with navigation", 1, 1)
{

    //addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
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

void Init2DMosaic::onNewImage(quint32 _port, Image &_image)
{
    qDebug() << logPrefix() << "Receive image on port " << _port;
    qDebug() << logPrefix() << "Process InitMatchModule";

    // Forward image
    postImage(0, _image);

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

void Init2DMosaic::onFlush(quint32 _port)
{
    qDebug() << logPrefix() << "flush port " << _port;

    emit si_processCompletion(0);
    emit si_userInformation("Initializing 2D Mosaic...");

    MosaicDescriptor *p_mosaic_d = new MosaicDescriptor;
    QVector<ProjectiveCamera*> *p_cams = new QVector<ProjectiveCamera*>;
    ImageSet * image_set;

    // Find imageSet corresponding to this port
    foreach (ImageSetPort *im_set,*m_input_port_list){
        if (im_set->port_number == _port){
            image_set = im_set->image_set;
            break;
        }
    }

    int images_width;

    if (image_set)
    {
        if(image_set->getNumberOfImages()>0)
        {
            QList<Image*> imageList = image_set->getAllImages();
            images_width = image_list[0]->width();
        }
        else
        {
            emit signal_showErrorMessage(tr("Did not find any image"), tr("Did not find any image."));
            return;
        }

    }
    else
    {
        emit si_showErrorMessage(tr("Did not find any image"), tr("Did not find any image."));
        return;
    }

    emit signal_processCompletion(10);

    bool Ok = false;

    // Get camera equipment
    CameraInfo camera_equipment = m_matisse_parameters->getCamInfoParamValue("cam_param", "camera_equipment", Ok);

    if (!Ok)
    {
        fatalErrorExit(tr("Did not found the camera equipment. Please check that the required equipment is available in the database."));
        return;
    }
   
    if (camera_equipment.cameraName() == "Unknown")
    {
        fatalErrorExit("It is not possible to use unknown camera equipment for 2D mosaicking.");
        return;
    }


    // Fill camera matrix
    int full_sensor_width, full_sensor_height;
    camera_equipment.fullSensorSize(full_sensor_width, full_sensor_height);

    cv::Mat K = camera_equipment.K();

    double on_run_scale_factor = m_matisse_parameters->getDoubleParamValue("algo_param", "scale_factor", Ok);

    // on run scale factor is already included in images_width so we remove it as it is handled by the optical mapping framework
    double scaling_factor_comp_to_calib = images_width / ((double)full_sensor_width*on_run_scale_factor); 
    K.at<double>(0, 0) = scaling_factor_comp_to_calib * K.at<double>(0, 0);
    K.at<double>(1, 1) = scaling_factor_comp_to_calib * K.at<double>(1, 1);
    K.at<double>(0, 2) = scaling_factor_comp_to_calib * K.at<double>(0, 2);
    K.at<double>(1, 2) = scaling_factor_comp_to_calib * K.at<double>(1, 2);


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

    if (image_set){

        // Create cameras
        QList<Image *> image_list = image_set->getAllImages();
        foreach (Image* image, image_list) {

            NavImage *nav_image = dynamic_cast<NavImage*>(image);
            if (nav_image) {
                if (navImage->navInfo().altitude()>0.0)
                {
                    p_cams->push_back(new ProjectiveCamera(nav_image, K, V_T_C, V_R_C, on_run_scale_factor));
                }
            }else{
                qDebug() << "cannot cast as navImage \n";
                exit(1);
            }
        }

        // Init cameras
        p_mosaic_d->initCamerasAndFrames(*p_cams,true);
        qDebug() << "Init done";

        // Filter cameras on overlap
        if (m_matisse_parameters->getBoolParamValue("algo_param", "disjoint_drawing", ok)) {
            p_mosaic_d->computeMosaicExtentAndShiftFrames();
            p_mosaic_d->decimateImagesUntilNoOverlap();
        }
        else if ( m_matisse_parameters->getBoolParamValue("algo_param","filter_overlap", ok) ){
            double min_overlap = m_matisse_parameters->getDoubleParamValue("algo_param","min_overlap", ok);
            double max_overlap = m_matisse_parameters->getDoubleParamValue("algo_param","max_overlap", ok);

            p_mosaic_d->computeMosaicExtentAndShiftFrames();
            p_mosaic_d->decimateImagesFromOverlap(min_overlap, max_overlap);
        }

        // Compute extents
        p_mosaic_d->computeMosaicExtentAndShiftFrames();
        qDebug() << "Extent computation done";

    }

    emit si_processCompletion(90);

    // Add pCams to mosaic _context
    QVariant * p_cams_stocker = new QVariant();
    p_cams_stocker->setValue(p_cams);
    m_context->addObject("Cameras", p_cams_stocker);

    // Add pMosaicD to mosaic _context
    QVariant * p_mosaic_d_stocker = new QVariant();
    p_mosaic_d_stocker->setValue(p_mosaic_d);
    m_context->addObject("MosaicDescriptor", p_mosaic_d_stocker);

    emit si_processCompletion(100);


    // Flush next module port
    flush(0);

}

} // namespace matisse

