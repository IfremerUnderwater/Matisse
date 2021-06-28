#include <QString>

#include "image.h"

using namespace cv;

namespace matisse_image {

Image::Image():m_id(-1),
    m_image_data(0)
{
}


Image::Image(const Image &_other, bool _shallow_copy): m_id(_other.m_id)
{
    Mat * image_data = _other.m_image_data;
    if (image_data) {
        if (_shallow_copy) {
            m_image_data = image_data;
        }
        else {
            m_image_data = new Mat(*image_data);
        }
    } else {
        m_image_data = new Mat();
    }
}


Image::Image(int _id, Mat *_image_data):m_id(_id)
{
    if (_image_data) {
        m_image_data = new Mat(*_image_data);
    } else {
        m_image_data = 0;
    }
}

Image::~Image() {
    delete m_image_data;
}

void Image::releaseImageData()
{
    if (m_image_data) {
        delete m_image_data;
        m_image_data = 0;
    }
}

int Image::width()
{
    if(m_image_data != 0){
        return m_image_data->cols;
    }else{
        return -1;
    }
}

int Image::height()
{
    if(m_image_data != 0){
        return m_image_data->rows;
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
