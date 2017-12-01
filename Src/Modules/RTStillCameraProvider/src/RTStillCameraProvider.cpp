#include "RTStillCameraProvider.h"
#include "Sleeper.h"
#include "Dim2.h"
#include "NavInfo.h"
#include "FileImage.h"
#include "GeoTransform.h"

//#ifdef WIN32
// TODO : hack windows dll....
//*******************************************
#include "NavPhotoInfoTcpListener.h"
//*******************************************
//#endif

#define D2R (3.14159265358979323846 / 180.0)

Q_EXPORT_PLUGIN2(RTStillCameraProvider, RTStillCameraProvider)

RTStillCameraProvider::RTStillCameraProvider(QObject *parent)
    : ImageProvider(NULL, "RTStillCameraProvider", "", 1),
      _pictureFileSet(NULL),
      _imageCount(0),
      _sensorFullWidth(0),
      _sensorFullHeight(0),
      _doRealTimeMosaicking(true)
{
    Q_UNUSED(parent)
    setIsRealTime(true);

    _imageSet = new ImageSet();

    addExpectedParameter("cam_param", "still_camera_address");
    addExpectedParameter("cam_param", "still_camera_port");
    addExpectedParameter("cam_param", "sensor_width");
    addExpectedParameter("cam_param", "sensor_height");
    addExpectedParameter("algo_param", "do_realtime_mosaicking");

    _imageDownloader = new HTTPImageDownloader();

    qRegisterMetaType<NavPhotoInfoMessage>();


    _navPhotoInfoTcpListener = new NavPhotoInfoTcpListener();
    connect(_navPhotoInfoTcpListener, SIGNAL(signal_NavPhotoInfoMessage(NavPhotoInfoMessage)), this, SLOT(slot_processNavPhotoInfoMessage(NavPhotoInfoMessage)), Qt::QueuedConnection);
    connect(this, SIGNAL(signal_connectTcpSocket(QString,int)), _navPhotoInfoTcpListener, SLOT(slot_Connect(QString,int)), Qt::QueuedConnection);
    connect(this, SIGNAL(signal_disconnectTcpSocket()), _navPhotoInfoTcpListener, SLOT(slot_disconnect()), Qt::QueuedConnection);

    _navPhotoInfoTcpListener->moveToThread(&_rtImagesListener);

    // Connect HTTP image downloader
    connect(_imageDownloader,SIGNAL(signal_imageReady(QImage)), this, SLOT(slot_onReceiveImage(QImage)));

    _rtImagesListener.start();

}

RTStillCameraProvider::~RTStillCameraProvider()
{
    delete _imageSet;
    _navPhotoInfoTcpListener->deleteLater();
    delete _imageDownloader;
}

ImageSet *RTStillCameraProvider::imageSet(quint16 port)
{
    Q_UNUSED(port)
    return _imageSet;
}

bool RTStillCameraProvider::configure()
{
    //QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    bool ok;
    _tcpPort = _matisseParameters->getIntParamValue("cam_param", "still_camera_port", ok );
    if (!ok) {
        return false;
    }

    _sensorFullWidth = _matisseParameters->getIntParamValue("cam_param", "sensor_width", ok );
    if (!ok) {
        return false;
    }

    _sensorFullHeight = _matisseParameters->getIntParamValue("cam_param", "sensor_height", ok );
    if (!ok) {
        return false;
    }

    _tcpAddress = _matisseParameters->getStringParamValue("cam_param", "still_camera_address");

    _doRealTimeMosaicking = _matisseParameters->getBoolParamValue("algo_param", "do_realtime_mosaicking", ok);
    if (!ok) {
        return false;
    }

    qDebug() << logPrefix()  << "TCP connection port: " << _tcpPort;

    // To be changed
    _refFrame = cv::Mat(_sensorFullHeight, _sensorFullWidth, CV_8UC3);

    return true;
}

bool RTStillCameraProvider::start()
{
    qDebug() << logPrefix() << "Started TCP Listening... ";
    emit signal_connectTcpSocket(_tcpAddress,_tcpPort);
    return true;
}



bool RTStillCameraProvider::stop()
{

    _imageSet->clear();
    emit signal_disconnectTcpSocket();
    _imageCount = 0;
    return true;
}


void RTStillCameraProvider::slot_processNavPhotoInfoMessage(NavPhotoInfoMessage msg_p)
{
    if (isStarted()) {
        if(msg_p.has_photopath()){

            QDateTime photoTime;
            photoTime.setMSecsSinceEpoch((msg_p.photostamp().sec())*1000000 + (uint64)(msg_p.photostamp().nsec()/1000));

            _lastNavInfo.setInfo(0,
                                 photoTime,
                                 msg_p.longitude(),
                                 msg_p.latitude(),
                                 msg_p.depth(),
                                 msg_p.altitude(),
                                 msg_p.yaw(),
                                 msg_p.roll(),
                                 msg_p.pitch(),
                                 0.0,
                                 0.0,
                                 0.0,
                                 D2R*msg_p.pan(),
                                 D2R*msg_p.tilt());

            QFileInfo photoFile(QString::fromStdString(msg_p.photopath()));
            QString photoBasename = photoFile.completeBaseName();
            if (photoFile.suffix() == "jpg"){
                photoBasename = photoBasename + ".jpg";
            }else{
                photoBasename = photoBasename + ".nef.jpg";
            }

            QString photoFileUrl = QString("http://") + _tcpAddress + QString(":8080/Preview/") + photoBasename;

            if (_doRealTimeMosaicking){
                _imageDownloader->startDownloadOfFile(photoFileUrl);
            }
            else{
                NavImage * newImage = new NavImage(_imageCount++, &_refFrame, _lastNavInfo);
                _imageSet->addImage(newImage);
            }


        }else{
            /*   GeoTransform T;
            QList<QgsPoint> navPoint;

            double utm_x,utm_y;
            QString utmZone;

            T.LatLongToUTM(msg_p.latitude(),msg_p.longitude(),utm_x,utm_y, utmZone);
qDebug() << "lat = " << msg_p.latitude() << "lon = " << msg_p.longitude() << "\n";
qDebug() << "utmX = " << utm_x << "utmY = " << utm_y << "\n";
            navPoint.append(QgsPoint(utm_x,utm_y));
            emit signal_addQGisPointsToMap(navPoint,"green","nav");*/
        }
    }
}

void RTStillCameraProvider::slot_onReceiveImage(QImage downImage_p)
{
    cv::Mat *downImageOpenCv = new cv::Mat;
    *downImageOpenCv = QImageToCvMat( downImage_p);

    NavImage * newImage = new NavImage(_imageCount++, downImageOpenCv, _lastNavInfo);

    _imageSet->addImage(newImage);
}

cv::Mat RTStillCameraProvider::QImageToCvMat( const QImage &inImage, bool inCloneImageData)
{
    switch ( inImage.format() )
    {
    // 8-bit, 4 channel
    case QImage::Format_RGB32:
    {
        cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

        return (inCloneImageData ? mat.clone() : mat);
    }

        // 8-bit, 3 channel
    case QImage::Format_RGB888:
    {
        if ( !inCloneImageData )
            qWarning() << "ASM::QImageToCvMat() - Conversion requires cloning since we use a temporary QImage";

        QImage   swapped = inImage.rgbSwapped();

        return cv::Mat( swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine() ).clone();
    }

        // 8-bit, 1 channel
    case QImage::Format_Indexed8:
    {
        cv::Mat  mat( inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine() );

        return (inCloneImageData ? mat.clone() : mat);
    }

    default:
        qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
        break;
    }

    return cv::Mat();
}



