#ifndef RTStillCameraProvider_H
#define RTStillCameraProvider_H

#include "ImageProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
//#include "Dim2UdpListener.h"
#include <QThread>

#include <QMetaType>
#include "NavPhotoInfoTcpListener.h"
#include "httpImageDownloader.h"
#include "NavInfo.h"

Q_DECLARE_METATYPE(NavPhotoInfoMessage)

using namespace MatisseCommon;
class Worker;

class RTStillCameraProvider :  public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "RTStillCameraProvider")
#endif

public:
    explicit RTStillCameraProvider(QObject *parent = 0);
    virtual ~RTStillCameraProvider();

    virtual ImageSet * imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();

signals:
    void signal_connectTcpSocket(QString hostname_p, int port_p);
    void signal_disconnectTcpSocket();

public slots:
    void slot_processNavPhotoInfoMessage(NavPhotoInfoMessage navPhotoInfoMsg_p);
    void slot_onReceiveImage(QImage downImage_p);

protected:
    cv::Mat QImageToCvMat(const QImage &inImage, bool inCloneImageData=true);

private:
    PictureFileSet * _pictureFileSet;
    int _imageCount;
    cv::Mat _refFrame;

    int _sensorFullWidth,_sensorFullHeight;

    bool _doRealTimeMosaicking;

    NavPhotoInfoTcpListener *_navPhotoInfoTcpListener;
    HTTPImageDownloader *_imageDownloader;
    ImageSet * _imageSet;
    QThread _rtImagesListener;

    QString _tcpAddress;
    int _tcpPort;
    NavInfo _lastNavInfo;

};

#endif // RTStillCameraProvider_H
