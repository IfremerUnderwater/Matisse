#ifndef RTVIDEODIRECTSHOWPROVIDER_H
#define RTVIDEODIRECTSHOWPROVIDER_H

#include "ImageProvider.h"
#include "ImageSet.h"
#include "CameraVideoSource.h"
#include "Dim2UdpListener.h"


using namespace MatisseCommon;
class WorkerVideo;

class RTVideoDirectShowProvider : public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)

public:
    explicit RTVideoDirectShowProvider(QObject *parent = 0);
    virtual ~RTVideoDirectShowProvider();

    virtual ImageSet* imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();

    void timerEvent(QTimer *evt);

private:
    CameraVideoSource* _cvCaptureCam;
    Dim2UDPListener* _dim2UdpListener;
    int _portUdpDim2Listener;
    int _captureInterval;
    int _idTimerCapture;
    QString _lastDim2Frame;
    ImageSet* _imageSet;
    WorkerVideo* _worker;
    bool _dim2Valid;


signals:
    void signal_previewStart();
    void signal_previewStop();

public slots:
    void dim2Received( QString dim2line );
    void captureImage();

};

#endif // RTVIDEDIRECTSHOWPROVIDER_H
