#include "CartoScene.h"
#include <QGraphicsItem>
#include <math.h>

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
}

void CartoScene::addCartoImage(CartoImage *image)
{
    Mat dest;

    int nbchannels = image->imageData()->channels();
    QImage::Format format = QImage::Format_RGB888;

    // only RGB and RGBA images can be used
    if(nbchannels == 3)
    {
        cvtColor(*(image->imageData()), dest, CV_BGR2RGB);
        format = QImage::Format_RGB888;
    }
    else if (nbchannels == 4)
    {
        // with alpha CV_BGRA2RGBA
        cvtColor(*(image->imageData()), dest, CV_BGRA2RGBA);
        format = QImage::Format_RGBA8888;
    }
    else
    {
        // error
        return;
    }

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, format);
    const QPixmap pix = QPixmap::fromImage(result);

    QRectF imageEnv = image->getEnvelope();

    // compute scale
    if(_scale == 1.0 && _parentSize.height() > 0 && _parentSize.width() > 0 && _envelope.width() == 0 && _envelope.height() == 0
            && imageEnv.width() > 0 && imageEnv.height() > 0)
    {
        // square pixels - ensure
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

    //pixmap boundaries
    // size in pixels : take values NOT rotated
    //QSize size(imageEnv.width(), imageEnv.height());
    QSize size(image->widthGeo() * _scale, image->heightGeo() * _scale);

    QGraphicsPixmapItem *pm = addPixmap(pix.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QRectF r(pm->boundingRect());
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

    // free memory here
    delete image;
}

void CartoScene::clearScene()
{
    clear();
    _envelope.setRect(0,0,0,0);
    _scale = 1.0;
    _rectEnv = NULL;
}

void CartoScene::setParentSize(QSize size)
{
    _parentSize = size;
}

void CartoScene::unionEnvelope(QRectF addRect)
{
    if(_envelope.width() == 0 && _envelope.height() == 0)
    {
        _envelope = addRect; //.normalized();
    }
    else
    {
        // compute
        _envelope = _envelope.united(addRect); //.normalized());
    }
}
