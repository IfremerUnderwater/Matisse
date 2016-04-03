#include <QDir>

#include "RTSurveyPlotter.h"


Q_EXPORT_PLUGIN2(RTSurveyPlotter, RTSurveyPlotter)

RTSurveyPlotter::RTSurveyPlotter(QObject *parent):
    RasterProvider(NULL, "RTSurveyPlotter", "", 1)
{
    Q_UNUSED(parent)

    addExpectedParameter("vehic_param", "alt_setpoint");
    addExpectedParameter("cam_param",  "K");
    //addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param", "sensor_width");
    addExpectedParameter("cam_param", "sensor_height");
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

    _sensorFullWidth = _matisseParameters->getIntParamValue("cam_param", "sensor_width", Ok );
    if (!Ok) {
        return false;
    }

    _sensorFullHeight = _matisseParameters->getIntParamValue("cam_param", "sensor_height", Ok );
    if (!Ok) {
        return false;
    }


    // for future use
    _scaleFactor = 1.0;

    // Get camera lever arm
    _V_T_C = cv::Mat(3,1,CV_64F);
    _V_R_C = cv::Mat(3,3,CV_64F);


    return true;
}

void RTSurveyPlotter::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;

    // Create camera
    NavImage *navImage = dynamic_cast<NavImage*>(&image);
    _imageList.append(navImage);

    // init scale factor
    if (navImage->id() == 0)
        _scaleFactor = (double)navImage->width()/(double)_sensorFullWidth;

    if (navImage){
        bool Ok;
        Matrix6x1 V_Pose_C = _matisseParameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  Ok);
        if (Ok){

            for (int i=0; i<3; i++){
                _V_T_C.at<qreal>(i,0) = V_Pose_C(0,i);
            }

            GeoTransform T;

            //_V_R_C = T.RotZ(V_Pose_C(0,5))*T.RotY(V_Pose_C(0,4)+(navImage->navInfo().tilt()-M_PI_2))*T.RotX(V_Pose_C(0,3)+navImage->navInfo().pan());
            _V_R_C = T.RotX(V_Pose_C(0,3))*T.RotY(V_Pose_C(0,4)+(navImage->navInfo().tilt()-M_PI_2))*T.RotZ(V_Pose_C(0,5)-navImage->navInfo().pan());

        }


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
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   0,   1), pt1, true);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, 0,   1), pt2, true);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << w-1, h-1, 1), pt3, true);
    _pCams->at(navImage->id())->projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   h-1, 1), pt4, true);

    // Construct polygon
    basicproc::Polygon camWireframe;
    std::vector<double> xArray,yArray;
    xArray.clear(); yArray.clear();

    xArray.push_back((pt1.at<qreal>(0,0)/pt1.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().x);
    yArray.push_back((-pt1.at<qreal>(1,0)/pt1.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().y);
    xArray.push_back((pt2.at<qreal>(0,0)/pt2.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().x);
    yArray.push_back((-pt2.at<qreal>(1,0)/pt2.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().y);
    xArray.push_back((pt3.at<qreal>(0,0)/pt3.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().x);
    yArray.push_back((-pt3.at<qreal>(1,0)/pt3.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().y);
    xArray.push_back((pt4.at<qreal>(0,0)/pt4.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().x);
    yArray.push_back((-pt4.at<qreal>(1,0)/pt4.at<qreal>(2,0))+_pMosaicD->mosaicOrigin().y);

    camWireframe.addContour(xArray,yArray);

    // Contruc Nav point
    QList<QgsPoint> navPoint;
    navPoint.append(QgsPoint(navImage->navInfo().utmX(), navImage->navInfo().utmY()));
    emit signal_addQGisPointsToMap(navPoint,"blue","nav");

    emit signal_addPolylineToMap(camWireframe,"red","imageswireframe");

    // Project Image
    cv::Mat warpedImage, warpedImageMask;
    cv::Point corner_p;
    _pCams->at(navImage->id())->projectImageOnMosaickingPlane(warpedImage, warpedImageMask, corner_p);

    MosaicDescriptor singleFrameMosaic;
    cv::Mat img_ullr;
    img_ullr = (cv::Mat_<qreal>(4,1) << _pMosaicD->mosaicOrigin().x + corner_p.x*(double)_pMosaicD->pixelSize().x,
    _pMosaicD->mosaicOrigin().y - corner_p.y*(double)_pMosaicD->pixelSize().y,
    _pMosaicD->mosaicOrigin().x + (corner_p.x+(double)warpedImage.cols-1.0)*_pMosaicD->pixelSize().x,
    _pMosaicD->mosaicOrigin().y - (corner_p.y+(double)warpedImage.rows-1.0)*_pMosaicD->pixelSize().y);

    singleFrameMosaic.setMosaic_ullr(img_ullr);
    singleFrameMosaic.setUtmZone(_pMosaicD->utmZone());
    singleFrameMosaic.setUtmHemisphere(_pMosaicD->utmHemisphere());

    //QString imgFile = QString("/home/data/DATA/RealTimeMosaic/image_%1.tiff").arg(navImage->id());
    QString imgFileTmp = QString("/home/aarnaube/image_temp_%1.tiff").arg(navImage->id());
    QString imgFile = QString("/home/aarnaube/image_%1.tiff").arg(navImage->id());

    // create image with tranparency
    std::vector<Mat> rasterChannels;
    cv::Mat warpedImageTransparent;
    split(warpedImage, rasterChannels);
    rasterChannels.pop_back();
    rasterChannels.push_back(warpedImageMask);
    merge(rasterChannels, warpedImageTransparent);

    cv::imwrite(imgFileTmp.toStdString(),warpedImageTransparent);

    QString utmProjParam, utmHemisphereOption;

    if ( _pMosaicD->utmHemisphere() == "S" ){
        utmHemisphereOption = QString(" +south");
    }else{
        utmHemisphereOption = QString("");
    }
    utmProjParam = QString("+proj=utm +zone=") + QString("%1").arg(_pMosaicD->utmZone());

    QString gdalOptions =  QString("-a_srs \"")+ utmProjParam + QString("\" -of GTiff -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
            .arg(img_ullr.at<qreal>(0,0),0,'f',2)
            .arg(img_ullr.at<qreal>(1,0),0,'f',2)
            .arg(img_ullr.at<qreal>(2,0),0,'f',2)
            .arg(img_ullr.at<qreal>(3,0),0,'f',2)
            + QString(" -mask 4 --config GDAL_TIFF_INTERNAL_MASK YES ");

    QString cmdLine;
    cmdLine = QString("gdal_translate ") + gdalOptions + " " + imgFileTmp + " " + imgFile;
    system(cmdLine.toStdString().c_str());

    qDebug() << "system cmd = " << cmdLine;
    QFile::remove(imgFileTmp);

    //if(navImage->id() == 3)
    emit signal_addRasterFileToMap(imgFile);
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

