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

    virtual bool configure(Context * context, MatisseParameters * mosaicParameters);
    virtual void onNewImage(quint32 port, Image &image);
    virtual void start();
    virtual void stop();
    virtual QFileInfo rasterInfo();

private:
    QFileInfo _outputFileInfo;

signals:
    
public slots:
    
};

#endif // GEOTIFFIMAGEWRITER_H
