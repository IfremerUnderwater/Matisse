#ifndef USERFORMWIDGET_H
#define USERFORMWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QImage>
#include <QGraphicsView>
#include <QThread>
#include <QListWidget>


#include <osg/ref_ptr>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>


Q_DECLARE_METATYPE(osg::ref_ptr<osg::Node>)


#if 0
#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
//#include <qgsmaplayerregistry.h>

#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
//#include <qgsmarkersymbollayerv2.h>
//#include <qgssinglesymbolrendererv2.h>
//#include <qgsrendererv2.h>
#include <qgsproject.h>
#include <qgsmapcanvas.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>
#include <qgscomposition.h>
#endif
//#include <opencv2/opencv.hpp>

#include "Image.h"
#include "Polygon.h"
#include "GraphicalCharter.h"
#include "MatisseIconFactory.h"
#include "IconizedActionWrapper.h"

#include "CartoScene.h"
#include "CartoImage.h"

using namespace MatisseCommon;
using namespace MatisseTools;

class OSGWidget;

namespace Ui {
class UserFormWidget;
}
//class QgsRasterLayer;

class QLabel;

enum CartoViewType { QGisMapLayer, QImageView, OpenSceneGraphView };

enum RepaintBehaviorState { ExtentAutoResize, FollowLastItem, ManualMove };


class resultLoadingTask : public QObject{
    Q_OBJECT

public:
    explicit resultLoadingTask();
    virtual ~resultLoadingTask();

    void setOSGWidget(OSGWidget* _osgwidget) { m_osgwidget = _osgwidget; }

signals:
//    void signal_addRasterToCartoView(QgsRasterLayer * rasterLayer_p);
    void signal_addRasterToCartoView(CartoImage  * image_p);
    void signal_addRasterToImageView(Image  * image_p);
    void signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p);

public slots:
    void slot_loadRasterFromFile(QString filename_p = "");
    void slot_load3DSceneFromFile(QString filename_p = "");

private:
    OSGWidget* m_osgwidget;


};

class UserFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit UserFormWidget(QWidget *parent = NULL);
    virtual ~UserFormWidget();

    void switchCartoViewTo(CartoViewType cartoViewType_p);

    void setIconFactory(MatisseIconFactory *iconFactory);
    void initCanvas();
    void initMapToolBar();
    void initLayersWidget();
    void clear();
    void displayImage(Image *image);
    void displayCartoImage(CartoImage *image);
    void resetJobForm();
    void loadRasterFile(QString filename);
    void loadShapefile(QString filename);
    void load3DFile(QString filename_p = "");
    void loadImageFile(QString filename);
    void saveQgisProject(QString filename);
    //void loadTestVectorLayer();
    //void addQGisPointsToMap(QList<QgsPoint> &pointsList_p, QString pointsColor_p, QString layerName_p);
    void addPolygonToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p);
    void addPolylineToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p);
    void exportMapViewToImage(QString imageFilePath);

    CartoViewType currentViewType() const;

    QStringList supportedRasterFormat() const;
    QStringList supportedVectorFormat() const;
    QStringList supported3DFileFormat() const;
    QStringList supportedImageFormat() const;

    void setLayersWidget(QListWidget *layersWidget);

protected slots:
    //void slot_addRasterToCartoView(QgsRasterLayer * rasterLayer_p);
    void slot_addRasterToCartoView(CartoImage *image_p);
    void slot_addRasterToImageView(Image *image_p);
    void slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p);
    void slot_showLayersWidgetContextMenu(const QPoint &pos);
    void slot_showMapContextMenu(const QPoint& pos_p);
    void slot_onAutoResizeTrigger();
    void slot_onFollowLastItem();
    void slot_onManualMove();
    void slot_updateColorPalette(QMap<QString,QString>);
    void slot_showHideToolbar();
    //void slot_activatePanTool();
    void slot_activateZoomInTool();
    void slot_activateZoomOutTool();
    void slot_recenterMap();
    void slot_showImagesRect(bool);

    //void slot_layerWasAdded(QgsMapLayer *layer);
    //void slot_layerWasRemoved(QString layerId);
    void slot_layerItemChanged();
    void slot_removeLayer();

    void slot_updateMapCoords(QPointF p);
    void slot_mapZoomChanged(qreal z);
    void slot_mapPanChanged();

signals:
    void signal_loadRasterFromFile(QString filename_p = "");
    void signal_load3DSceneFromFile(QString filename_p = "");

private:
    //void updateMapCanvasAndExtent(QgsMapLayer *currentLayer_p);
    bool findLayerIndexFromName(const QString &layerName_p, int &idx_p);

    Ui::UserFormWidget *_ui;
    //QList<QgsMapCanvasLayer> _layers;
    // QGis 2.99
   // QList<QgsMapLayer*> _players;
    // attention : pointeur - delete à faire

    CartoScene _scene;

    bool _isToolBarDisplayed;

    MatisseIconFactory *_iconFactory;

    CartoViewType _currentViewType;

    QThread _resultLoadingThread;
    resultLoadingTask _resultLoadingTask;

    QStringList _supportedRasterFormat;
    QStringList _supportedVectorFormat;
    QStringList _supported3DFileFormat;
    QStringList _supportedImageFormat;

    QAction *_extentAutoResize;
    QAction *_followLastItem;
    QAction *_manualMove;
    QMenu *_repaintBehaviorMenu;

    QAction *_removeLayerAction;
    QMenu *_layersMenu;

    RepaintBehaviorState _repaintBehaviorState;
    QToolBar *_mapToolBar;
    QLabel *_coords;

    //QgsMapTool *_panTool;
    //QgsMapTool *_zoomInTool;
    //QgsMapTool *_zoomOutTool;

    QListWidget *_layersWidget;
};


#endif // USERFORMWIDGET_H
