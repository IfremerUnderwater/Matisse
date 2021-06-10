#include "init_2d_mosaic.h"
#include "nav_image.h"

#include "mosaic_descriptor.h"

#include "Polygon.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init2DMosaic, Init2DMosaic)
#endif

using namespace nav_tools;
using namespace optical_mapping;

namespace matisse {

Init2DMosaic::Init2DMosaic() :
    Processor(NULL, "Init2DMosaic", "Init 2D mosaic Descriptor with navigation", 1, 1)
{

    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param",  "V_Pose_C");

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

    emit si_processCompletion(10);

    // Get camera matrix
    bool ok;
    QMatrix3x3 q_K = m_matisse_parameters->getMatrix3x3ParamValue("cam_param",  "K",  ok);
    cv::Mat K(3,3,CV_64F);
    if (ok){
        qDebug() << "K Ok =" << ok;

        for (int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                K.at<double>(i,j) = q_K(i,j);
            }
        }

    }else{
        qDebug() << "K Ok =" << ok;
        exit(1);
    }

    std::cerr <<"K = " << K;

    emit si_processCompletion(30);

    double scale_factor = m_matisse_parameters->getDoubleParamValue("algo_param", "scale_factor", ok);

    if (!ok){
        qDebug() << "scale factor not provided Ok \n";
        exit(1);
    }

    // Get camera lever arm
    cv::Mat V_T_C(3,1,CV_64F), V_R_C(3,3,CV_64F);

    Matrix6x1 V_Pose_C = m_matisse_parameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  ok);
    if (ok){

        for (int i=0; i<3; i++){
            V_T_C.at<double>(i,0) = V_Pose_C(0,i);
        }

        GeoTransform T;

        V_R_C = T.rotZ(V_Pose_C(0,5))*T.rotY(V_Pose_C(0,4))*T.rotX(V_Pose_C(0,3));

    }

    emit si_processCompletion(60);

    if (image_set){

        // Create cameras
        QList<Image *> image_list = image_set->getAllImages();
        foreach (Image* image, image_list) {

            NavImage *nav_image = dynamic_cast<NavImage*>(image);
            if (nav_image) {
                if (nav_image->navInfo().altitude()>0.0)
                {
                    p_cams->push_back(new ProjectiveCamera(nav_image, K, V_T_C, V_R_C, (double)scale_factor));
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

