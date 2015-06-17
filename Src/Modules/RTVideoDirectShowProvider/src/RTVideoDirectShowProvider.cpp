#include <QDir>
#include "WorkerVideo.h"
#include "RTVideoDirectShowProvider.h"


Q_EXPORT_PLUGIN2(RTVideoDirectShowProvider, RTVideoDirectShowProvider)

RTVideoDirectShowProvider::RTVideoDirectShowProvider(QObject *parent):
    ImageProvider(NULL, "RTVideoDirectShowProvider", "", 1),
    _worker(NULL), _captureInterval(1000), _idTimerCapture(-1), _dim2Valid(false)
{
    setIsRealTime(true);
    _imageSet = new ImageSet();

    addExpectedParameter("video_input_param", "idCamera");
    addExpectedParameter("dim2_param", "udp_port");
    addExpectedParameter("video_input_param", "intervalCapture");


    _cvCaptureCam = new CameraVideoSource(0, 1 );
    connect( this, SIGNAL(signal_previewStart()), _cvCaptureCam, SLOT(slot_previewStart()));
    connect( this, SIGNAL(signal_previewStop()), _cvCaptureCam, SLOT(slot_previewStop()));

    _cvCaptureCam->configure();

}

RTVideoDirectShowProvider::~RTVideoDirectShowProvider()
{
    qDebug() << "Delete RTVideoDirectShowProvider";
    delete _cvCaptureCam;
}


ImageSet * RTVideoDirectShowProvider::imageSet(quint16 port)
{
    return _imageSet;
}

bool RTVideoDirectShowProvider::configure()
{
    bool ret = true;
    qDebug() << logPrefix() << "RTVideoDirectShowProvider configure";

    /// Création et initialisation du device de capture video
    ///
    ///
    bool ok;
    int idCamera =  _matisseParameters->getIntParamValue( "video_input_param", "idCamera", ok );
    int interval = _matisseParameters->getIntParamValue( "video_input_param", "intervalCapture", ok );

    return ret;

}

bool RTVideoDirectShowProvider::start()
{
    qDebug() << logPrefix() << " Begin start";

    connect( _cvCaptureCam, SIGNAL(imageReady()), this, SLOT(captureImage()));


    ///
    /// Création et initialisation du listener de trame udp dim2 (geoposisionnement)
    ///
    bool ok;
    _portUdpDim2Listener = _matisseParameters->getIntParamValue("dim2_param", "udp_port", ok );
    _dim2UdpListener = new Dim2UDPListener();
    _dim2UdpListener->slot_configure(_portUdpDim2Listener);

    _worker = new WorkerVideo(this);
    _worker->moveToThread(QThread::currentThread());
    connect( _dim2UdpListener, SIGNAL(signal_newline(QString)), _worker, SLOT(slot_processLine(QString)), Qt::QueuedConnection);

    emit signal_previewStart();
    bool ret = _cvCaptureCam->start();

    qDebug() << logPrefix() << " End start";

    return ret;
}

bool RTVideoDirectShowProvider::stop()
{
    _imageSet->clear();

    emit signal_previewStop();
    delete _dim2UdpListener;
    delete _worker;
    return true;
}

void RTVideoDirectShowProvider::dim2Received(QString dim2Frame)
{
    qDebug() << logPrefix() << "Recevied : " << dim2Frame;
    _lastDim2Frame.clear();
    _lastDim2Frame = dim2Frame;
    captureImage();
    _dim2Valid = true;
}

void RTVideoDirectShowProvider::captureImage()
{
    qDebug() << logPrefix() << "captureImage ";
    if( isStarted() && _dim2Valid ){
        _imageSet->clear();
        NavImage * image = _cvCaptureCam->captureImage(_lastDim2Frame);
        if( image != NULL )
        {
            _imageSet->addImage(image);
            _dim2Valid = false;
            _imageSet->flush();
        }
    }
}


