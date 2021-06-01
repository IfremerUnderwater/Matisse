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

#include "image.h"
#include "Polygon.h"
#include "graphical_charter.h"
#include "matisse_icon_factory.h"
#include "iconized_action_wrapper.h"

#include "carto_scene.h"
#include "carto_image.h"
#include <vector>

using namespace MatisseTools;
using namespace nav_tools;

class OSGWidget;

namespace Ui {
class DataViewer;
}

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
    void signal_addRasterToCartoView(CartoImage  * image_p);
    void signal_addRasterToImageView(Image  * image_p);
    void signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p, bool remove_previous_scenes_p);

public slots:
    void slot_loadRasterFromFile(QString filename_p = "");
    void slot_load3DSceneFromFile(QString filename_p, bool remove_previous_scenes_p);

private:
    OSGWidget* m_osgwidget;


};

class DataViewer : public QWidget
{
    Q_OBJECT
    
public:
    explicit DataViewer(QWidget *parent = NULL);
    virtual ~DataViewer();

    void switchCartoViewTo(CartoViewType cartoViewType_p);

    void setIconFactory(MatisseIconFactory *iconFactory);
    void initCanvas();
    void initMapToolBar();
    void clear();
    void displayImage(Image *image);
    void displayCartoImage(CartoImage *image);
    void resetJobForm();
    void loadRasterFile(QString filename);
    void load3DFile(QString filename_p, bool remove_previous_scenes_p=true);
    void loadImageFile(QString filename);
    void exportMapViewToImage(QString imageFilePath);

    CartoViewType currentViewType() const;

    QStringList supportedRasterFormat() const;
    QStringList supportedVectorFormat() const;
    QStringList supported3DFileFormat() const;
    QStringList supportedImageFormat() const;

protected slots:
    void slot_addRasterToCartoView(CartoImage *image_p);
    void slot_addRasterToImageView(Image *image_p);
    void slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p, bool _remove_previous_scenes=true);
    void slot_showMapContextMenu(const QPoint& pos_p);
    void slot_onAutoResizeTrigger();
    void slot_onFollowLastItem();
    void slot_onManualMove();
    void slot_updateColorPalette(QMap<QString,QString>);
    void slot_showHideToolbar();
    void slot_activateZoomInTool();
    void slot_activateZoomOutTool();
    void slot_recenterMap();
    void slot_showImagesRect(bool);


    void slot_updateMapCoords(QPointF p);
    void slot_mapZoomChanged(qreal z);
    void slot_mapPanChanged();

signals:
    void signal_loadRasterFromFile(QString filename_p = "");
    void signal_load3DSceneFromFile(QString filename_p, bool remove_previous_scenes_p);

private:

    Ui::DataViewer *_ui;

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

    RepaintBehaviorState _repaintBehaviorState;
    QToolBar *_mapToolBar;
    QLabel *_coords;

    std::vector< osg::ref_ptr<osg::Node> > _osg_nodes;

};


#endif // USERFORMWIDGET_H
