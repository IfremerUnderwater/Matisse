#include "image_set.h"
#include <QDebug>

namespace matisse_image {

Image *ImageSet::getImage(int _image_number)
{

        Image * ret_image = 0;
        if (m_images.size() > _image_number) {
            ret_image = m_images[_image_number];
            if (m_drop_images) {
                m_images.removeAt(_image_number);
            }
        }

        return ret_image;


}

QList<Image *> ImageSet::getAllImages()
{
    return m_images;
}

QList<Image *> ImageSet::getImages(int _size, int _image_number)
{
    QList<Image *> ret_images;
    int max_index = _image_number + _size;
    if (m_images.size() > max_index) {
        ret_images = m_images.mid(_image_number, _size);
        if (m_drop_images) {
            for(int index = _image_number; index < max_index; index++)
            {
                m_images.removeAt(index);
            }
        }
    }
    return ret_images;
}

int ImageSet::getNumberOfImages()
{
    return m_images.size();
}

int ImageSet::getMaxSize()
{
    return m_max_size;
}

bool ImageSet::addImage(Image *_image)
{
    bool ret = true;
    if (m_max_size > -1) {
        if (m_images.size() < m_max_size) {
            m_images.append(_image);
        } else if (m_drop_images){
            m_images.removeFirst();
            m_images.append(_image);
        } else {
            ret = false;
        }
    } else {
        m_images.append(_image);
    }

    if (m_out_port) {
        ImageListener *listener = m_out_port->image_listener;
        listener->onNewImage(m_out_port->port_number, *_image);
    } else {
        qDebug() << "ADD IMAGE: pas de port attache";
    }

    return ret;
}

bool ImageSet::addImages(QList<Image *> _images)
{
    bool ret = true;
    int count = _images.size();
    if (m_max_size > -1) {
        if ((m_images.size() + count) > m_max_size) {
            if (m_drop_images) {
                if (count > m_max_size) {
                    ret = false;
                } else {
                    count -= m_max_size - m_images.size();
                    while (count > 0) {
                        m_images.removeFirst();
                        count --;
                    }
                    m_images.append(_images);
                }
            } else {
                ret = false;
            }

        } else {
            m_images.append(_images);
        }

    } else {
        m_images.append(_images);
    }

    return ret;
}

void ImageSet::flush()
{
    if (m_out_port) {
        ImageListener *listener = m_out_port->image_listener;
        listener->onFlush(m_out_port->port_number);
    } else {
        qDebug() << "FLUSH: pas de port attache";
    }
}

void ImageSet::clear()
{
    m_images.clear();
}


QString ImageSet::dumpAttr()
{
    QString ret("ImageSet:\n");
    ret += QString("\tsize = %1\n").arg(m_images.size());
    ret += QString("\tmaxSize = %1\n").arg(m_max_size);
    ret += QString("\tdropImages = %1\n").arg(m_drop_images);

    return ret;
}
ImageSetPort *ImageSet::outPort() const
{
    return m_out_port;
}

void ImageSet::setOutPort(ImageSetPort *outPort)
{
    m_out_port = outPort;
}

} // namespace matisse_image
