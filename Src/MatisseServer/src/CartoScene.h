#ifndef CARTOSCENE_H
#define CARTOSCENE_H

#include <QGraphicsView>

#include "Image.h"
#include "CartoImage.h"

class QGraphicsRectItem;

class CartoScene : public QGraphicsScene
{
public:
    CartoScene();

    // add geo referenced image
    void addCartoImage(CartoImage *image);

    // replacement for clear()
    // reset envelope
    void clearScene();
    // must be called immediately after clearScene(=
    void setParentSize(QSize size);

private:
    // in geographical units top down
    QRectF _envelope;
    void unionEnvelope(QRectF addRect);

    QGraphicsRectItem *_rectEnv;

    // scale
    qreal _scale;

    // size (in pixel) of parent widget : to compute scale on first image load;
    QSize _parentSize;
};

#endif // CARTOSCENE_H
