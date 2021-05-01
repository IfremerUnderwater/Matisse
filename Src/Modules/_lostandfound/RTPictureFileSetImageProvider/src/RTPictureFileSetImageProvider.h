#ifndef RTPICTUREFILESETIMAGEPROVIDER_H
#define RTPICTUREFILESETIMAGEPROVIDER_H

#include "input_data_provider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "Dim2UdpListener.h"


using namespace MatisseCommon;
class Worker;

class RTPictureFileSetImageProvider :  public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "RTPictureFileSetImageProvider")
#endif

public:
    explicit RTPictureFileSetImageProvider(QObject *parent = 0);
    virtual ~RTPictureFileSetImageProvider();

    virtual ImageSet * imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();

    void processLine(QString line);

private:
    PictureFileSet * _pictureFileSet;
    int _imageCount;
    //QFileSystemWatcher* _watcher;
    //Dim2FileReader *_dim2FileReader;
    Dim2UDPListener *_udpListener;
    ImageSet * _imageSet;
    //QStringList _fileList;
    Worker* _worker;



};

#endif // RTPICTUREFILESETIMAGEPROVIDER_H
