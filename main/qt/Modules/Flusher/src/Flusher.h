#ifndef FLUSHER_H
#define FLUSHER_H


#include "ImageProvider.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "ImageSet.h"
#include "FileImage.h"

using namespace MatisseCommon;

class Flusher : public ImageProvider
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageProvider)
public:
    explicit Flusher(QObject *parent = 0);
    virtual ~Flusher();

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

#endif // FLUSHER_H
