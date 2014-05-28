#ifndef DTPICTUREFILESETIMAGEPROVIDER_H
#define DTPICTUREFILESETIMAGEPROVIDER_H


#include "ImageProvider.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

using namespace MatisseCommon;

class DTPictureFileSetImageProvider : public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)
public:
    explicit DTPictureFileSetImageProvider(QObject *parent = 0);
    virtual ~DTPictureFileSetImageProvider();

    virtual ImageSet * imageSet(quint16 port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();


private:
    PictureFileSet * _pictureFileSet;
    Dim2FileReader *_dim2FileReader;
    ImageSet * _imageSet;

signals:
    
public slots:
    
};

#endif // DTPICTUREFILESETIMAGEPROVIDER_H
