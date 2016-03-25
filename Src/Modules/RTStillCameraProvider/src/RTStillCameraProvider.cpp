#include "RTStillCameraProvider.h"
#include "Sleeper.h"
#include "Dim2.h"
#include "NavInfo.h"
#include "FileImage.h"

#define D2R (3.14159265358979323846 / 180.0)

Q_EXPORT_PLUGIN2(RTStillCameraProvider, RTStillCameraProvider)

RTStillCameraProvider::RTStillCameraProvider(QObject *parent)
    : ImageProvider(NULL, "RTStillCameraProvider", "", 1),
      _pictureFileSet(NULL),
      _imageCount(0)
{
    Q_UNUSED(parent)
    setIsRealTime(true);

    _imageSet = new ImageSet();

    addExpectedParameter("cam_param", "still_camera_address");
    addExpectedParameter("cam_param", "still_camera_port");
}

RTStillCameraProvider::~RTStillCameraProvider()
{
    delete _imageSet;
    delete _udpListener;
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
    int udpPort = _matisseParameters->getIntParamValue("cam_param", "still_camera_port", ok );
    if (!ok) {
        return false;
    }
    qDebug() << logPrefix()  << "Port: " << udpPort;
    //_pictureFileSet = new PictureFileSet(rootDirnameStr,false);

    // To be changed
    _refFrame = cv::Mat(1024,1024,CV_8UC3);

    _udpListener = new Dim2UDPListener();
    _udpListener->slot_configure(udpPort);

    connect(_udpListener, SIGNAL(signal_newline(QString)), this, SLOT(slot_processLine(QString)), Qt::QueuedConnection);
    _udpListener->moveToThread(&_rtImagesListener);

    _rtImagesListener.start();

    return true;
}

bool RTStillCameraProvider::start()
{
    qDebug() << logPrefix() << "Started... ";
    return true;
}



bool RTStillCameraProvider::stop()
{

    _imageSet->clear();
    _udpListener->deleteLater();
    //_rtImagesListener.deleteLater();
    _imageCount = 0;
    return true;
}


void RTStillCameraProvider::slot_processLine(QString dim2String)
{
    if (isStarted()) {
        qDebug() << "Receive: " << dim2String;
        Dim2 dim2(dim2String);
        NavInfo navInfo;
        navInfo.setInfo(dim2.diveNumber(),
                        dim2.dateTime(),
                        dim2.longitude(),
                        dim2.latitude(),
                        dim2.depth(),
                        dim2.altitude(),
                        D2R*dim2.yaw(),
                        D2R*dim2.roll(),
                        D2R*dim2.pitch(),
                        dim2.vx(),
                        dim2.vy(),
                        dim2.vz());

        NavImage * newImage = new NavImage(_imageCount++, &_refFrame, navInfo);
        _imageSet->addImage(newImage);
    }
}


