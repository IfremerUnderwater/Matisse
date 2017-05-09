#include "RTPictureFileSetImageProvider.h"
#include "Sleeper.h"
#include "Dim2.h"
#include "Worker.h"
#include "NavInfo.h"
#include "FileImage.h"


Q_EXPORT_PLUGIN2(RTPictureFileSetImageProvider, RTPictureFileSetImageProvider)

RTPictureFileSetImageProvider::RTPictureFileSetImageProvider(QObject *parent)
    : ImageProvider(NULL, "RTPictureFileSetImageProvider", "", 1),
      _pictureFileSet(NULL),
      _imageCount(0)
{
    Q_UNUSED(parent)
    setIsRealTime(true);


    _imageSet = new ImageSet();
    //_watcher=new QFileSystemWatcher();
    //connect(_watcher, SIGNAL(directoryChanged(QString)), this, SLOT(slot_directoryChanged(QString)));
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dim2_param", "udp_port");
}

RTPictureFileSetImageProvider::~RTPictureFileSetImageProvider()
{
    delete _imageSet;
    //delete _watcher;
}

ImageSet *RTPictureFileSetImageProvider::imageSet(quint16 port)
{
    Q_UNUSED(port)
    return _imageSet;
}

bool RTPictureFileSetImageProvider::configure()
{
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    bool ok;
    int udpPort = _matisseParameters->getIntParamValue("dim2_param", "udp_port", ok );
    if (!ok) {
        return false;
    }
    qDebug() << logPrefix()  << "Port: " << udpPort;
    _pictureFileSet = new PictureFileSet(rootDirnameStr);

    _udpListener = new Dim2UDPListener();
    _udpListener->slot_configure(udpPort);

    _worker = new Worker(this);
    _worker->moveToThread(QThread::currentThread());
    connect(_udpListener, SIGNAL(signal_newline(QString)), _worker, SLOT(slot_processLine(QString)), Qt::QueuedConnection);

    return true;
}

bool RTPictureFileSetImageProvider::start()
{
    qDebug() << logPrefix() << "Started... ";
    return true;
}



bool RTPictureFileSetImageProvider::stop()
{
    //_watcher->removePath(_pictureFileSet->rootDirname());
    //_fileList.clear();
    _imageSet->clear();
    _udpListener->deleteLater();
    _worker->deleteLater();
    _imageCount = 0;
    return true;
}


void RTPictureFileSetImageProvider::processLine(QString dim2String)
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
                        dim2.yaw(),
                        dim2.roll(),
                        dim2.pitch(),
                        dim2.vx(),
                        dim2.vy(),
                        dim2.vz());
        QString filename = dim2.filename();
        QString fileSource = dim2.source();
        QString fileFormat = dim2.format();
        FileImage * newImage = new FileImage(_pictureFileSet, filename, fileSource, fileFormat, _imageCount++, navInfo);
        _imageSet->addImage(newImage);
    }
}
/*
//
// Monitoring de répertoire
//
void RTPictureFileSetImageProvider::slot_directoryChanged(QString path)
{
    qDebug() << "Event in" << path;

    QStringList nameFilter("*.*");
    QDir directory(_pictureFileSet->rootDirname());
    QStringList newFiles  = directory.entryList(nameFilter);
    QStringList deletedFiles;

    QStringList savedList = newFiles;

    foreach (QString file, _fileList) {
        bool isAbsent= !newFiles.removeOne(file);
        if (isAbsent) {
            deletedFiles.append(file);
        }
    }

    // A ce point, newFiles ne contient que les nouveaux fichiers
    foreach (QString file, newFiles) {
        qDebug() << "Nouveau: " << file;
        FileImage * newImage = new FileImage(_pictureFileSet, file, fileSource, fileFormat, i, navInfo);
        _imageSet->addImage(newImage);
    }

    foreach (QString file, deletedFiles) {
        qDebug() << "Supprime: " << file;
    }

    _fileList = savedList;

}
*/



