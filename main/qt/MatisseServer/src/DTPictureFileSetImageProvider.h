#ifndef DTPICTUREFILESETIMAGEPROVIDER_H
#define DTPICTUREFILESETIMAGEPROVIDER_H

#include <QSharedPointer>

#include "ImageProvider.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

class DTPictureFileSetImageProvider : public ImageProvider
{
    Q_OBJECT
public:
    explicit DTPictureFileSetImageProvider(QObject *parent = 0);
    virtual ~DTPictureFileSetImageProvider();

    virtual ImageSet * getImageSet();
    virtual bool assemble(Context & context, MosaicParameters & mosaicParameters);
    virtual void start();
    virtual void stop();


private:
    PictureFileSet * _pictureFileSet;
    Dim2FileReader *_dim2FileReader;
    ImageSet * _imageSet;

signals:
    
public slots:
    
};

#endif // DTPICTUREFILESETIMAGEPROVIDER_H
