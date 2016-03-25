#include <QDir>

#include "RTSurveyPlotter.h"


Q_EXPORT_PLUGIN2(RTSurveyPlotter, RTSurveyPlotter)

RTSurveyPlotter::RTSurveyPlotter(QObject *parent):
    RasterProvider(NULL, "RTSurveyPlotter", "", 1)
{
    Q_UNUSED(parent)

    addExpectedParameter("vehic_param", "alt_setpoint");
    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param",  "V_Pose_C");
}

RTSurveyPlotter::~RTSurveyPlotter()
{
    qDebug() << logPrefix() << "Destroy RTSurveyPlotter";
}

bool RTSurveyPlotter::configure()
{
    qDebug() << logPrefix() << "configure";

    // No raster to write, only real time plotting
    _rastersInfo.clear();


    _pMosaicD = new MosaicDescriptor;
    _pCams = new QVector<ProjectiveCamera*>;


    // Get camera matrix
    bool Ok;
    QMatrix3x3 qK = _matisseParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
    _K = cv::Mat(3,3,CV_64F);
    if (Ok){
        qDebug() << "K Ok =" << Ok;

        for (int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                _K.at<qreal>(i,j) = qK(i,j);
            }
        }

    }else{
        qDebug() << "K Ok =" << Ok;
        return false;
    }

    std::cerr <<"K = " << _K;


    // for future use
    _scaleFactor = 1.0;

    // Get camera lever arm
    _V_T_C = cv::Mat(3,1,CV_64F);
    _V_R_C = cv::Mat(3,3,CV_64F);

    Matrix6x1 V_Pose_C = _matisseParameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  Ok);
    if (Ok){

        for (int i=0; i<3; i++){
            _V_T_C.at<qreal>(i,0) = V_Pose_C(0,i);
        }

        GeoTransform T;

        _V_R_C = T.RotZ(V_Pose_C(0,5))*T.RotY(V_Pose_C(0,4))*T.RotX(V_Pose_C(0,3));

    }


    return true;
}

void RTSurveyPlotter::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;

    // Create camera
    NavImage *navImage = dynamic_cast<NavImage*>(&image);
    _imageList.append(navImage);
    if (navImage){
        _pCams->push_back(new ProjectiveCamera(navImage , _K, _V_T_C, _V_R_C, (qreal)_scaleFactor));
    }else{
        qDebug() << "cannot cast as navImage \n";
        exit(1);
    }


    // Compute cameras
    if (navImage->id() == 0){
        _pMosaicD->initCamerasAndFrames(*_pCams,true);
        _utmZone = QString::number(_pMosaicD->utmZone()) + QString(" ") + _pMosaicD->utmHemisphere();
        qDebug() << "Init done";
    }else{

        GeoTransform T;
        qreal X,Y;
        T.LatLongToUTM(_pCams->at(navImage->id())->image()->navInfo().latitude(),
                       _pCams->at(navImage->id())->image()->navInfo().longitude(),
                       X, Y, _utmZone, true);


        // Affect UTM values
        _pCams->at(navImage->id())->image()->navInfo().setUtmX(X);
        _pCams->at(navImage->id())->image()->navInfo().setUtmY(Y);
        _pCams->at(navImage->id())->image()->navInfo().setUtmZone(_utmZone);

        _pMosaicD->computeCameraHomography(_pCams->at(navImage->id()));

    }


    // Compute corners
    int w = _pCams->at(navImage->id())->image()->width();
    int h = _pCams->at(navImage->id())->image()->height();
    cv::Mat pt1,pt2,pt3,pt4;

    // Project corners_p on mosaic plane
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   0,   1), pt1);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, 0,   1), pt2);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, h-1, 1), pt3);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   h-1, 1), pt4);

    // Construct polygon
    basicproc::Polygon camWireframe;
    std::vector<double> xArray,yArray;
    xArray.clear(); yArray.clear();

    xArray.push_back(pt1.at<qreal>(0,0)/pt1.at<qreal>(2,0));
    yArray.push_back(-pt1.at<qreal>(1,0)/pt1.at<qreal>(2,0));
    xArray.push_back(pt2.at<qreal>(0,0)/pt2.at<qreal>(2,0));
    yArray.push_back(-pt2.at<qreal>(1,0)/pt2.at<qreal>(2,0));
    xArray.push_back(pt3.at<qreal>(0,0)/pt3.at<qreal>(2,0));
    yArray.push_back(-pt3.at<qreal>(1,0)/pt3.at<qreal>(2,0));
    xArray.push_back(pt4.at<qreal>(0,0)/pt4.at<qreal>(2,0));
    yArray.push_back(-pt4.at<qreal>(1,0)/pt4.at<qreal>(2,0));

    camWireframe.addContour(xArray,yArray);

    emit signal_addPolygonToMap(camWireframe,"red","tictic");

}

void RTSurveyPlotter::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "Flush on port " << port;
}

bool RTSurveyPlotter::start()
{
    qDebug() << logPrefix() << " inside start";

    qDebug() << logPrefix() << " out start";
    return true;
}

bool RTSurveyPlotter::stop()
{
    return true;
}

QList<QFileInfo> RTSurveyPlotter::rastersInfo()
{
    return _rastersInfo;
}

