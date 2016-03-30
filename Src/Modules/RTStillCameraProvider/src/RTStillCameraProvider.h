#ifndef RTStillCameraProvider_H
#define RTStillCameraProvider_H

#include "ImageProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
//#include "Dim2UdpListener.h"
#include <QThread>
#include "NavPhotoInfoTcpListener.h"

#include <QMetaType>
#include "NavPhotoInfoTcpListener.h"

Q_DECLARE_METATYPE(NavPhotoInfoMessage)

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

signals:
    void signal_connectTcpSocket(QString hostname_p, int port_p);

public slots:
    void slot_processNavPhotoInfoMessage(NavPhotoInfoMessage navPhotoInfoMsg_p);

private:
    PictureFileSet * _pictureFileSet;
    int _imageCount;
    cv::Mat _refFrame;

    int m_sensorFullWidth,m_sensorFullHeight;

    NavPhotoInfoTcpListener *_navPhotoInfoTcpListener;
    ImageSet * _imageSet;
    QThread _rtImagesListener;

};

#endif // RTStillCameraProvider_H
