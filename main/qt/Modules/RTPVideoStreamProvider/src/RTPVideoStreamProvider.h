#ifndef RTPVIDEOSTREAMPROVIDER_H
#define RTPVIDEOSTREAMPROVIDER_H

#include "ImageProvider.h"
#include "ImageSet.h"
#include "CameraVideoSource.h"
#include "Dim2UdpListener.h"


using namespace MatisseCommon;
class WorkerVideo;

class RTPVideoStreamProvider : public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)

public:
    explicit RTPVideoStreamProvider(QObject *parent = 0);
    virtual ~RTPVideoStreamProvider();

    virtual ImageSet* imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();


private:
    CameraVideoSource* _cvCaptureCam;
    Dim2UDPListener* _dim2UdpListener;
    int _portUdpDim2Listener;

    ImageSet* _imageSet;
    WorkerVideo* _worker;

signals:
    
public slots:
    void dim2Received( QString dim2line );

};

#endif // RTPVIDEOSTREAMPROVIDER_H
