#include "carto_scene.h"
#include <QGraphicsItem>
#include <math.h>
#include <QDebug>
#include <QApplication>

// case of buggy gcc........
#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

using namespace cv;
using namespace nav_tools;

namespace matisse {

CartoScene::CartoScene() : QGraphicsScene(/*QObject *parent = Q_NULLPTR)*/)
{
    m_envelope.setRect(0,0,0,0);
    m_parent_size = QSize(0,0);
    m_scale = 1.0;
    m_rect_env = NULL;
    m_scale_factor = 1.0;
    m_show_images_rect = false;
    m_show_bounding_rect = false;
}

void CartoScene::addCartoImage(CartoImage *_image)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    Mat dest;

    int nbchannels = _image->imageData()->channels();
    QImage::Format format = QImage::Format_RGB888;

    // only RGB and RGBA images can be used
    if(nbchannels == 3)
    {
        cvtColor(*(_image->imageData()), dest, COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
    }
    else if (nbchannels == 4)
    {
        // with alpha CV_BGRA2RGBA
        cvtColor(*(_image->imageData()), dest, COLOR_BGRA2RGBA);
        format = QImage::Format_RGBA8888;
    }
    else
    {
        // error
        QApplication::restoreOverrideCursor();
        return;
    }

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, format);
    if(result.isNull() && nbchannels == 4)
    {
        qDebug() << "Error loading 4 channels data - trying 3 channels : " << _image->fileName();
        dest.release();
        cvtColor(*(_image->imageData()), dest, COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
        result = QImage((uchar*) dest.data, dest.cols, dest.rows, dest.step, format);
    }
    const QPixmap pix = QPixmap::fromImage(result);

    QRectF image_env = _image->getEnvelope();

    // compute scale
    if(m_scale == 1.0 && m_parent_size.height() > 0 && m_parent_size.width() > 0 && m_envelope.width() == 0 && m_envelope.height() == 0
            && image_env.width() > 0 && image_env.height() > 0)
    {
        // square pixels - ensure image fits in parent
        qreal w = image_env.width();
        qreal h = image_env.height();
        m_scale = min(m_parent_size.width() / w, m_parent_size.height() / h);
    }

    // scale rectangle
    image_env = QRectF(image_env.left() * m_scale,
                      image_env.top() * m_scale,
                      image_env.width() * m_scale,
                      image_env.height() * m_scale);

    // compute envelope
    unionEnvelope(image_env);

    //resize scene
    setSceneRect(m_envelope);

    // debug
    QRectF sr(m_envelope);
    if(m_rect_env == NULL)
    {
        QPen spen(QColor(255,255,0));
        m_rect_env = addRect(sr,spen);
    }
    else
    {
        m_rect_env->setRect(sr);
    }

    m_rect_env->hide();

    //pixmap boundaries
    // size in pixels : take values NOT rotated
    QSize size(_image->widthGeo() * m_scale, _image->heightGeo() * m_scale);
    // apply scale factor
    QSize size_pix(_image->widthGeo() * m_scale * m_scale_factor, _image->heightGeo() * m_scale *m_scale_factor);
    // limit to pixels numbers in X
    qreal sfactor = m_scale_factor;
    if(size_pix.width() > _image->width())
    {
        size_pix = QSize(_image->width(), _image->height());
        sfactor = ((qreal)_image->width()) / (_image->widthGeo() * m_scale);
    }
    QGraphicsPixmapItem *pm = addPixmap(pix.scaled(size_pix, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pm->setToolTip(_image->fileName());
    pm->setScale(1.0 / sfactor );

    QRectF r(QPointF(0,0),size);
    QPen pen(QColor(255,0,0));
    QGraphicsRectItem *rect = addRect(r,pen);
    // compute position (scale applied)
    qreal x_pos = _image->xGeo(0,0) * m_scale;
    qreal y_pos = _image->yGeo(0,0) * m_scale;;

    // rotation
    qreal angle = _image->getRotationAngle();
    // transform in QT world
    //Sets the clockwise rotation angle, in degrees, around the Z axis. The default value is 0 (i.e., the item is not rotated).
    // Assigning a negative value will rotate the item counter-clockwise.
    pm->setTransformOriginPoint(0,0);
    pm->setRotation( - angle * 180.0 / M_PI);

    rect->setTransformOriginPoint(0,0);
    rect->setRotation( - angle * 180.0 / M_PI);

    // set position
    pm->setPos(x_pos, y_pos);
    rect->setPos(x_pos, y_pos);

    rect->hide();

    // free memory here
    _image->releaseImageData();

    m_image_list.append(new ZoomableImage(_image->fileName(), pm, rect, sfactor, sfactor != m_scale_factor));

    delete _image;

    QApplication::restoreOverrideCursor();
}

void CartoScene::clearScene()
{
    clear();
    m_envelope.setRect(0,0,0,0);
    m_scale = 1.0;
    m_rect_env = NULL;

    // delete objects
    QList<ZoomableImage*>::iterator it = m_image_list.begin();
    for ( ; it != m_image_list.end(); ++it )
    {
        ZoomableImage *zi = *it;
        delete zi;
    }
    m_image_list.clear();

    m_scale_factor = 1.0;
}

void CartoScene::setParentSize(QSize _size)
{
    m_parent_size = _size;
}

void CartoScene::setScaleFactor(qreal _val)
{
    m_scale_factor = _val;
}

void CartoScene::unionEnvelope(QRectF _add_rect)
{
    if(m_envelope.width() == 0 && m_envelope.height() == 0)
    {
        m_envelope = _add_rect;
    }
    else
    {
        // compute
        m_envelope = m_envelope.united(_add_rect);
    }
}

void CartoScene::reloadVisibleImageWithNewScaleFactor(QGraphicsView * _view)
{
    QList<QGraphicsItem *> items = _view->items(_view->viewport()->rect());
    QList<QGraphicsItem *>::iterator vit = items.begin();
    for ( ; vit != items.end(); ++vit )
    {
        QGraphicsItem *i = *vit;
        // there is only one pixmap / ZoomableImage
        // rects are ignored
        if(i->type() != QGraphicsPixmapItem::Type)
            continue;

        QList<ZoomableImage*>::iterator it = m_image_list.begin();
        for ( ; it != m_image_list.end(); ++it )
        {
            ZoomableImage *zi = *it;
            if( i == zi->pixmap() && zi->scaleFactor() < scaleFactor() && !zi->maxScale())
            {
                qDebug() << " reload " << zi->fileName() << " scalefactor " << zi->scaleFactor() << " -> " << scaleFactor();
                // process
                removeItem(zi->pixmap());
                removeItem(zi->rect());
                m_image_list.removeOne(zi);

                CartoImage *ci = new CartoImage();
                ci->loadFile(zi->fileName());

                addCartoImage(ci);

                delete zi;

                break;
            }
        }
    }
}


// display image rectangles (default : false)
void CartoScene::showImageRect(const bool _show)
{
    m_show_images_rect = _show;

    QList<ZoomableImage*>::iterator it = m_image_list.begin();
    for ( ; it != m_image_list.end(); ++it )
    {
        ZoomableImage *zi = *it;
        if(_show)
            zi->rect()->show();
        else
            zi->rect()->hide();
    }
}

// display scene bounding rectangle (default : false)
void CartoScene::showSceneBoundingRectangle(const bool _show)
{
    m_show_bounding_rect = _show;
    if(m_rect_env != NULL)
    {
        if(_show)
            m_rect_env->show();
        else
            m_rect_env->hide();
    }
}

} // namespace matisse
