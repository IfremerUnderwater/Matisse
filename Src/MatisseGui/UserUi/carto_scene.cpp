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

CartoScene::CartoScene() : QGraphicsScene(/*QObject *parent = Q_NULLPTR)*/)
{
    _envelope.setRect(0,0,0,0);
    _parentSize = QSize(0,0);
    _scale = 1.0;
    _rectEnv = NULL;
    _scaleFactor = 1.0;
    _showImagesRect = false;
    _showBoundingRect = false;
}

void CartoScene::addCartoImage(CartoImage *image)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    Mat dest;

    int nbchannels = image->imageData()->channels();
    QImage::Format format = QImage::Format_RGB888;

    // only RGB and RGBA images can be used
    if(nbchannels == 3)
    {
        cvtColor(*(image->imageData()), dest, COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
    }
    else if (nbchannels == 4)
    {
        // with alpha CV_BGRA2RGBA
        cvtColor(*(image->imageData()), dest, COLOR_BGRA2RGBA);
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
        qDebug() << "Error loading 4 channels data - trying 3 channels : " << image->fileName();
        dest.release();
        cvtColor(*(image->imageData()), dest, COLOR_BGR2RGB);
        format = QImage::Format_RGB888;
        result = QImage((uchar*) dest.data, dest.cols, dest.rows, dest.step, format);
    }
    const QPixmap pix = QPixmap::fromImage(result);

    QRectF imageEnv = image->getEnvelope();

    // compute scale
    if(_scale == 1.0 && _parentSize.height() > 0 && _parentSize.width() > 0 && _envelope.width() == 0 && _envelope.height() == 0
            && imageEnv.width() > 0 && imageEnv.height() > 0)
    {
        // square pixels - ensure image fits in parent
        qreal w = imageEnv.width();
        qreal h = imageEnv.height();
        _scale = min(_parentSize.width() / w, _parentSize.height() / h);
    }

    // scale rectangle
    imageEnv = QRectF(imageEnv.left() * _scale,
                      imageEnv.top() * _scale,
                      imageEnv.width() * _scale,
                      imageEnv.height() * _scale);

    // compute envelope
    unionEnvelope(imageEnv);

    //resize scene
    setSceneRect(_envelope);

    // debug
    QRectF sr(_envelope);
    if(_rectEnv == NULL)
    {
        QPen spen(QColor(255,255,0));
        _rectEnv = addRect(sr,spen);
    }
    else
    {
        _rectEnv->setRect(sr);
    }

    _rectEnv->hide();

    //pixmap boundaries
    // size in pixels : take values NOT rotated
    QSize size(image->widthGeo() * _scale, image->heightGeo() * _scale);
    // apply scale factor
    QSize sizePix(image->widthGeo() * _scale * _scaleFactor, image->heightGeo() * _scale *_scaleFactor);
    // limit to pixels numbers in X
    qreal sfactor = _scaleFactor;
    if(sizePix.width() > image->width())
    {
        sizePix = QSize(image->width(), image->height());
        sfactor = ((qreal)image->width()) / (image->widthGeo() * _scale);
    }
    QGraphicsPixmapItem *pm = addPixmap(pix.scaled(sizePix, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pm->setToolTip(image->fileName());
    pm->setScale(1.0 / sfactor );

    QRectF r(QPointF(0,0),size);
    QPen pen(QColor(255,0,0));
    QGraphicsRectItem *rect = addRect(r,pen);
    // compute position (scale applied)
    qreal xPos = image->xGeo(0,0) * _scale;
    qreal yPos = image->yGeo(0,0) * _scale;;

    // rotation
    qreal angle = image->getRotationAngle();
    // transform in QT world
    //Sets the clockwise rotation angle, in degrees, around the Z axis. The default value is 0 (i.e., the item is not rotated).
    // Assigning a negative value will rotate the item counter-clockwise.
    pm->setTransformOriginPoint(0,0);
    pm->setRotation( - angle * 180.0 / M_PI);

    rect->setTransformOriginPoint(0,0);
    rect->setRotation( - angle * 180.0 / M_PI);

    // set position
    pm->setPos(xPos, yPos);
    rect->setPos(xPos, yPos);

    rect->hide();

    // free memory here
    image->releaseImageData();

    _imageList.append(new ZoomableImage(image->fileName(), pm, rect, sfactor, sfactor != _scaleFactor));

    delete image;

    QApplication::restoreOverrideCursor();
}

void CartoScene::clearScene()
{
    clear();
    _envelope.setRect(0,0,0,0);
    _scale = 1.0;
    _rectEnv = NULL;

    // delete objects
    QList<ZoomableImage*>::iterator it = _imageList.begin();
    for ( ; it != _imageList.end(); ++it )
    {
        ZoomableImage *zi = *it;
        delete zi;
    }
    _imageList.clear();

    _scaleFactor = 1.0;
}

void CartoScene::setParentSize(QSize size)
{
    _parentSize = size;
}

void CartoScene::setScaleFactor(qreal val)
{
    _scaleFactor = val;
}

void CartoScene::unionEnvelope(QRectF addRect)
{
    if(_envelope.width() == 0 && _envelope.height() == 0)
    {
        _envelope = addRect;
    }
    else
    {
        // compute
        _envelope = _envelope.united(addRect);
    }
}

void CartoScene::reloadVisibleImageWithNewScaleFactor(QGraphicsView * view)
{
    QList<QGraphicsItem *> items = view->items(view->viewport()->rect());
    QList<QGraphicsItem *>::iterator vit = items.begin();
    for ( ; vit != items.end(); ++vit )
    {
        QGraphicsItem *i = *vit;
        // there is only one pixmap / ZoomableImage
        // rects are ignored
        if(i->type() != QGraphicsPixmapItem::Type)
            continue;

        QList<ZoomableImage*>::iterator it = _imageList.begin();
        for ( ; it != _imageList.end(); ++it )
        {
            ZoomableImage *zi = *it;
            if( i == zi->pixmap() && zi->scaleFactor() < scaleFactor() && !zi->maxScale())
            {
                qDebug() << " reload " << zi->fileName() << " scalefactor " << zi->scaleFactor() << " -> " << scaleFactor();
                // process
                removeItem(zi->pixmap());
                removeItem(zi->rect());
                _imageList.removeOne(zi);

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
void CartoScene::showImageRect(const bool show)
{
    _showImagesRect = show;

    QList<ZoomableImage*>::iterator it = _imageList.begin();
    for ( ; it != _imageList.end(); ++it )
    {
        ZoomableImage *zi = *it;
        if(show)
            zi->rect()->show();
        else
            zi->rect()->hide();
    }
}

// display scene bounding rectangle (default : false)
void CartoScene::showSceneBoundingRectangle(const bool show)
{
    _showBoundingRect = show;
    if(_rectEnv != NULL)
    {
        if(show)
            _rectEnv->show();
        else
            _rectEnv->hide();
    }
}
