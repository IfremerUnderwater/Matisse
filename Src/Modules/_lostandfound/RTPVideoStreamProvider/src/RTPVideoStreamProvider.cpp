#include <QDir>
#include "WorkerVideo.h"
#include "RTPVideoStreamProvider.h"


Q_EXPORT_PLUGIN2(RTPVideoStreamProvider, RTPVideoStreamProvider)

RTPVideoStreamProvider::RTPVideoStreamProvider(QObject *parent):
    ImageProvider(NULL, "RTPVideoStreamProvider", "", 1),_worker(NULL)
{
    setIsRealTime(true);
    _imageSet = new ImageSet();

    addExpectedParameter("video_input_param", "idCamera");
    addExpectedParameter("video_input_param", "udpDim2listenerPort");

}

RTPVideoStreamProvider::~RTPVideoStreamProvider()
{

}


ImageSet * RTPVideoStreamProvider::imageSet(quint16 port)
{
    return _imageSet;
}

bool RTPVideoStreamProvider::configure()
{
    bool ret = true;
    qDebug() << logPrefix() << "RTPVideoStreamProvider configure";

    /// Création et initialisation du device de capture video
    ///
    ///
    bool ok;
    int idCamera =  _matisseParameters->getIntParamValue( "video_input_param", "idCamera", ok );
    _cvCaptureCam = new CameraVideoSource(idCamera);
    ret = _cvCaptureCam->configure();
    ///

    /// Création et initialisation du listener de trame udp dim2 (geoposisionnement)
    ///
    _portUdpDim2Listener = _matisseParameters->getIntParamValue( "video_input_param", "udpDim2listenerPort", ok );
    _dim2UdpListener = new Dim2UDPListener();
    _dim2UdpListener->slot_configure(_portUdpDim2Listener);

    _worker = new WorkerVideo(this);
    _worker->moveToThread(QThread::currentThread());
    connect( _dim2UdpListener, SIGNAL(signal_newline(QString)), _worker, SLOT(slot_processLine(QString)), Qt::QueuedConnection);

    return ret;

}

bool RTPVideoStreamProvider::start()
{
    qDebug() << logPrefix() << " inside start";

    _cvCaptureCam->start();

    qDebug() << logPrefix() << " out start";
    return true;
}

bool RTPVideoStreamProvider::stop()
{
    _imageSet->clear();
    return true;
}

void RTPVideoStreamProvider::dim2Received(QString dim2Frame)
{
    if( isStarted()){
//        qDebug() << "Dim2 recieved" << dim2Frame;
        _imageSet->clear();
        NavImage * image = _cvCaptureCam->captureImage(dim2Frame);
        _imageSet->addImage(image);
        _imageSet->flush();

    }
}

