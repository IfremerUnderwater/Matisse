#ifndef RTStillCameraProvider_H
#define RTStillCameraProvider_H

#include "ImageProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "Dim2UdpListener.h"
#include <QThread>


using namespace MatisseCommon;
class Worker;

class RTStillCameraProvider :  public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)

public:
    explicit RTStillCameraProvider(QObject *parent = 0);
    virtual ~RTStillCameraProvider();

    virtual ImageSet * imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();

public slots:
    void slot_processLine(QString line);

private:
    PictureFileSet * _pictureFileSet;
    int _imageCount;
    cv::Mat _refFrame;

    Dim2UDPListener *_udpListener;
    ImageSet * _imageSet;
    QThread _rtImagesListener;



};

#endif // RTStillCameraProvider_H
