#ifndef GEOTIFFIMAGEWRITER_H
#define GEOTIFFIMAGEWRITER_H


#include "RasterProvider.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

using namespace MatisseCommon;

class GeoTiffImageWriter : public RasterProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::RasterProvider)
public:
    explicit GeoTiffImageWriter(QObject *parent = 0);
    virtual ~GeoTiffImageWriter();

    virtual void onNewImage(quint32 port, Image &image);
    virtual void onFlush(quint32 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> _rastersInfo;

signals:
    
public slots:
    
};

#endif // GEOTIFFIMAGEWRITER_H
