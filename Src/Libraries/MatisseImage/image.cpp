#include <QString>

#include "image.h"

using namespace cv;

namespace matisse_image {

Image::Image():_id(-1),
    _imageData(0)
{
}


Image::Image(const Image &other, bool shallowCopy): _id(other._id)
{
    Mat * imageData = other._imageData;
    if (imageData) {
        if (shallowCopy) {
            _imageData = imageData;
        }
        else {
            _imageData = new Mat(*imageData);
        }
    } else {
        _imageData = new Mat();
    }
}


Image::Image(int id, Mat *imageData):_id(id)
{
    if (imageData) {
        _imageData = new Mat(*imageData);
    } else {
        _imageData = 0;
    }
}

Image::~Image() {
    delete _imageData;
}

void Image::releaseImageData()
{
    if (_imageData) {
        delete _imageData;
        _imageData = 0;
    }
}

int Image::width()
{
    if(_imageData != 0){
        return _imageData->cols;
    }else{
        return -1;
    }
}

int Image::height()
{
    if(_imageData != 0){
        return _imageData->rows;
    }else{
        return -1;
    }
}


QString Image::dumpAttr()
{
    QString ret("Image:\n");
    ret += QString("\tId = %1\n").arg(id());
    ret += QString("\tImage ptr = %1\n").arg(quint64(imageData()));

    return ret;

}

} // namespace matisse_image
