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
      _imageCount(0),
      m_sensorFullWidth(0),
      m_sensorFullHeight(0)
{
    Q_UNUSED(parent)
    setIsRealTime(true);

    _imageSet = new ImageSet();

    addExpectedParameter("cam_param", "still_camera_address");
    addExpectedParameter("cam_param", "still_camera_port");
    addExpectedParameter("cam_param", "sensor_width");
    addExpectedParameter("cam_param", "sensor_height");

    qRegisterMetaType<NavPhotoInfoMessage>();
}

RTStillCameraProvider::~RTStillCameraProvider()
{
    delete _imageSet;
    _navPhotoInfoTcpListener->deleteLater();
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
    int tcpPort = _matisseParameters->getIntParamValue("cam_param", "still_camera_port", ok );
    if (!ok) {
        return false;
    }

    m_sensorFullWidth = _matisseParameters->getIntParamValue("cam_param", "sensor_width", ok );
    if (!ok) {
        return false;
    }

    m_sensorFullHeight = _matisseParameters->getIntParamValue("cam_param", "sensor_height", ok );
    if (!ok) {
        return false;
    }

    QString tcpAddress = _matisseParameters->getStringParamValue("cam_param", "still_camera_address");

    qDebug() << logPrefix()  << "TCP connection port: " << tcpPort;

    // To be changed
    _refFrame = cv::Mat(m_sensorFullHeight, m_sensorFullWidth, CV_8UC3);

    _navPhotoInfoTcpListener = new NavPhotoInfoTcpListener();
    connect(_navPhotoInfoTcpListener, SIGNAL(signal_NavPhotoInfoMessage(NavPhotoInfoMessage)), this, SLOT(slot_processNavPhotoInfoMessage(NavPhotoInfoMessage)), Qt::QueuedConnection);
    connect(this, SIGNAL(signal_connectTcpSocket(QString,int)), _navPhotoInfoTcpListener, SLOT(slot_Connect(QString,int)), Qt::QueuedConnection);

    _navPhotoInfoTcpListener->moveToThread(&_rtImagesListener);

    emit signal_connectTcpSocket(tcpAddress,tcpPort);

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

    _imageCount = 0;
    return true;
}


void RTStillCameraProvider::slot_processNavPhotoInfoMessage(NavPhotoInfoMessage msg_p)
{
    if (isStarted()) {
        if(msg_p.has_photopath()){

            QDateTime photoTime;
            photoTime.setMSecsSinceEpoch((msg_p.photostamp().sec())*1000000 + (uint64)(msg_p.photostamp().nsec()/1000));

            NavInfo navInfo;
            navInfo.setInfo(0,
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
                            msg_p.pan(),
                            msg_p.tilt());

            NavImage * newImage = new NavImage(_imageCount++, &_refFrame, navInfo);
            _imageSet->addImage(newImage);
        }
    }
}


