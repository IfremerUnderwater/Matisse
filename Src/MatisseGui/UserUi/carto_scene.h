#ifndef MATISSE_CARTO_SCENE_H_
#define MATISSE_CARTO_SCENE_H_

#include <QGraphicsView>

#include "image.h"
#include "carto_image.h"

class QGraphicsRectItem;
class QGraphicsPixmapItem;

namespace matisse {

class CartoScene : public QGraphicsScene
{
    class ZoomableImage
    {
        QGraphicsPixmapItem *m_pm;
        QGraphicsRectItem *m_rect;
        qreal m_scale_factor;
        QString m_filename;
        bool m_max_scale;
    public:
        ZoomableImage(QString _filename, QGraphicsPixmapItem *_pm, QGraphicsRectItem *_rect, qreal _inscale_fcator, bool _max_scale) :
            m_pm(_pm), m_rect(_rect), m_scale_factor(_inscale_fcator), m_filename(_filename), m_max_scale(_max_scale) {}

        QGraphicsPixmapItem* pixmap() const { return m_pm; }
        QGraphicsRectItem* rect() const { return m_rect; }
        QString fileName() const { return m_filename; }
        qreal scaleFactor() const { return m_scale_factor; }
        bool maxScale() const { return m_max_scale; }

        void setPixmap(QGraphicsPixmapItem *_pm ) { m_pm = _pm; }
        void setRect( QGraphicsRectItem *_rect) {m_rect = _rect;}
    };

public:
    CartoScene();

    // add geo referenced image
    void addCartoImage(nav_tools::CartoImage *_image);

    void reloadVisibleImageWithNewScaleFactor(QGraphicsView * _view);

    // replacement for clear()
    // reset envelope
    void clearScene();
    // must be called immediately after clearScene()
    void setParentSize(QSize _size);

    // possibility of changing scale factor used for pixmaps
    // processed only for next loaded images, including images reloaded
    // with reloadVisibleImageWithNewScaleFactor
    void setScaleFactor(qreal _val);
    qreal scaleFactor() const { return m_scale_factor; }

    qreal scale() const { return m_scale; }

    // display image rectangles (default : false)
    void showImageRect(const bool _show);

    // display scene bounding rectangle (default : false)
    void showSceneBoundingRectangle(const bool _show);

private:
    // in geographical units top down
    QRectF m_envelope;
    void unionEnvelope(QRectF _add_rect);

    QGraphicsRectItem *m_rect_env;

    // scale
    qreal m_scale;
    // scale factor
    qreal m_scale_factor;

    // size (in pixel) of parent widget : to compute scale on first image load;
    QSize m_parent_size;

    // list of pixmaps
    QList<ZoomableImage*> m_image_list;

    bool m_show_images_rect;
    bool m_show_bounding_rect;

};

} // namespace matisse

#endif // MATISSE_CARTO_SCENE_H_
