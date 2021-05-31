#include "image_set.h"
#include <QDebug>

namespace matisse_image {

Image *ImageSet::getImage(int imageNumber)
{

        Image * retImage = 0;
        if (_images.size() > imageNumber) {
            retImage = _images[imageNumber];
            if (_dropImages) {
                _images.removeAt(imageNumber);
            }
        }

        return retImage;


}

QList<Image *> ImageSet::getAllImages()
{
    return _images;
}

QList<Image *> ImageSet::getImages(int size, int imageNumber)
{
    QList<Image *> retImages;
    int maxIndex = imageNumber + size;
    if (_images.size() > maxIndex) {
        retImages = _images.mid(imageNumber, size);
        if (_dropImages) {
            for(int index = imageNumber; index < maxIndex; index++)
            {
                _images.removeAt(index);
            }
        }
    }
    return retImages;
}

int ImageSet::getNumberOfImages()
{
    return _images.size();
}

int ImageSet::getMaxSize()
{
    return _maxSize;
}

bool ImageSet::addImage(Image *image)
{
    bool ret = true;
    if (_maxSize > -1) {
        if (_images.size() < _maxSize) {
            _images.append(image);
        } else if (_dropImages){
            _images.removeFirst();
            _images.append(image);
        } else {
            ret = false;
        }
    } else {
        _images.append(image);
    }

    if (_outPort) {
        ImageListener *listener = _outPort->imageListener;
        listener->onNewImage(_outPort->portNumber, *image);
    } else {
        qDebug() << "ADD IMAGE: pas de port attache";
    }

    return ret;
}

bool ImageSet::addImages(QList<Image *> images)
{
    bool ret = true;
    int count = images.size();
    if (_maxSize > -1) {
        if ((_images.size() + count) > _maxSize) {
            if (_dropImages) {
                if (count > _maxSize) {
                    ret = false;
                } else {
                    count -= _maxSize - _images.size();
                    while (count > 0) {
                        _images.removeFirst();
                        count --;
                    }
                    _images.append(images);
                }
            } else {
                ret = false;
            }

        } else {
            _images.append(images);
        }

    } else {
        _images.append(images);
    }

    return ret;
}

void ImageSet::flush()
{
    if (_outPort) {
        ImageListener *listener = _outPort->imageListener;
        listener->onFlush(_outPort->portNumber);
    } else {
        qDebug() << "FLUSH: pas de port attache";
    }
}

void ImageSet::clear()
{
    _images.clear();
}


QString ImageSet::dumpAttr()
{
    QString ret("ImageSet:\n");
    ret += QString("\tsize = %1\n").arg(_images.size());
    ret += QString("\tmaxSize = %1\n").arg(_maxSize);
    ret += QString("\tdropImages = %1\n").arg(_dropImages);

    return ret;
}
ImageSetPort *ImageSet::outPort() const
{
    return _outPort;
}

void ImageSet::setOutPort(ImageSetPort *outPort)
{
    _outPort = outPort;
}

} // namespace matisse_image
