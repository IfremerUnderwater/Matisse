#ifndef CARTOSCENE_H
#define CARTOSCENE_H

#include <QGraphicsView>

#include "Image.h"
#include "CartoImage.h"

class QGraphicsRectItem;
class QGraphicsPixmapItem;

class CartoScene : public QGraphicsScene
{
    class ZoomableImage
    {
        QGraphicsPixmapItem *_pm;
        QGraphicsRectItem *_rect;
        qreal _scaleFactor;
        QString _fileName;
        bool _maxScale;
    public:
        ZoomableImage(QString fileName, QGraphicsPixmapItem *pm, QGraphicsRectItem *rect, qreal inscaleFcator, bool maxScale) :
            _pm(pm), _rect(rect), _scaleFactor(inscaleFcator), _fileName(fileName), _maxScale(maxScale) {}

        QGraphicsPixmapItem* pixmap() const { return _pm; }
        QGraphicsRectItem* rect() const { return _rect; }
        QString fileName() const { return _fileName; }
        qreal scaleFactor() const { return _scaleFactor; }
        bool maxScale() const { return _maxScale; }

        void setPixmap(QGraphicsPixmapItem *pm ) { _pm = pm; }
        void setRect( QGraphicsRectItem *rect) {_rect = rect;}
    };

public:
    CartoScene();

    // add geo referenced image
    void addCartoImage(CartoImage *image);

    void reloadVisibleImageWithNewScaleFactor(QGraphicsView * view);

    // replacement for clear()
    // reset envelope
    void clearScene();
    // must be called immediately after clearScene()
    void setParentSize(QSize size);

    // possibility of changing scale factor used for pixmaps
    // processed only for next loaded images, including images reloaded
    // with reloadVisibleImageWithNewScaleFactor
    void setScaleFactor(qreal val);
    qreal scaleFactor() const { return _scaleFactor; }

    qreal scale() const { return _scale; }

    // display image rectangles (default : false)
    void showImageRect(const bool show);

    // display scene bounding rectangle (default : false)
    void showSceneBoundingRectangle(const bool show);

private:
    // in geographical units top down
    QRectF _envelope;
    void unionEnvelope(QRectF addRect);

    QGraphicsRectItem *_rectEnv;

    // scale
    qreal _scale;
    // scale factor
    qreal _scaleFactor;

    // size (in pixel) of parent widget : to compute scale on first image load;
    QSize _parentSize;

    // list of pixmaps
    QList<ZoomableImage*> _imageList;

    bool _showImagesRect;
    bool _showBoundingRect;

};

#endif // CARTOSCENE_H
