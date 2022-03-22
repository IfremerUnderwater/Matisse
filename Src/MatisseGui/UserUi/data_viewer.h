#ifndef MATISSE_DATA_VIEWER_H_
#define MATISSE_DATA_VIEWER_H_

#include <QWidget>
#include <QMenu>
#include <QToolBar>
#include <QImage>
#include <QGraphicsView>
#include <QThread>
#include <QListWidget>
#include <QLabel>


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
#include <QTimer>

using namespace nav_tools;


namespace Ui {
class DataViewer;
}


namespace matisse {

class OSGWidget;

enum eCartoViewType { QGIS_MAP_LAYER, QIMAGE_VIEW, OPEN_SCENE_GRAPH_VIEW };

enum eRepaintBehaviorState { EXTENT_AUTO_RESIZE, FOLLOW_LAST_ITEM, MANUAL_MOVE };


class resultLoadingTask : public QObject{
    Q_OBJECT

public:
    explicit resultLoadingTask();
    virtual ~resultLoadingTask();

    void setOSGWidget(OSGWidget* _osgwidget) { m_osgwidget = _osgwidget; }

signals:
    void si_addRasterToCartoView(CartoImage  * _image_p);
    void si_addRasterToImageView(Image  * _image_p);
    void si_add3DSceneToCartoView(osg::ref_ptr<osg::Node> _scene_data_p, bool _remove_previous_scenes_p, bool _reset_view);

public slots:
    void sl_loadRasterFromFile(QString _filename_p = "");
    void sl_load3DSceneFromFile(QString _filename_p, bool _remove_previous_scenes_p, bool _reset_view);

private:
    OSGWidget* m_osgwidget;


};

class DataViewer : public QWidget
{
    Q_OBJECT
    
public:
    explicit DataViewer(QWidget *_parent = NULL);
    virtual ~DataViewer();

    void switchCartoViewTo(eCartoViewType _carto_view_type_p);

    void setIconFactory(MatisseIconFactory *_icon_factory);
    void initCanvas();
    void initMapToolBar();
    void clear();
    void displayImage(Image *_image);
    void displayCartoImage(CartoImage *_image);
    void resetJobForm();
    void loadRasterFile(QString _filename);
    void invokeThreaded3DFileLoader(QString _filename_p, bool _remove_previous_scenes_p=true, bool _reset_view=true);
    void autoAdd3DFileFromFolderOnMainView(QString _folderpath_p);
    void loadImageFile(QString _filename);
    void exportMapViewToImage(QString _image_file_path);

    eCartoViewType currentViewType() const;

    QStringList supportedRasterFormat() const;
    QStringList supportedVectorFormat() const;
    QStringList supported3DFileFormat() const;
    QStringList supportedImageFormat() const;

protected slots:
    void sl_addRasterToCartoView(CartoImage *_image_p);
    void sl_addRasterToImageView(Image *_image_p);
    void sl_add3DSceneToCartoView(osg::ref_ptr<osg::Node> _scene_data_p, bool _remove_previous_scenes=true, bool _reset_view=true);
    void sl_checkFor3DFiles();
    void sl_showMapContextMenu(const QPoint& _pos_p);
    void sl_onAutoResizeTrigger();
    void sl_onFollowLastItem();
    void sl_onManualMove();
    void sl_updateColorPalette(QMap<QString,QString>);
    void sl_showHideToolbar();
    void sl_activateZoomInTool();
    void sl_activateZoomOutTool();
    void sl_recenterMap();
    void sl_showImagesRect(bool);


    void sl_updateMapCoords(QPointF _p);
    void sl_mapZoomChanged(qreal _z);
    void sl_mapPanChanged();

signals:
    void si_loadRasterFromFile(QString _filename_p = "");
    void si_load3DSceneFromFile(QString _filename_p, bool _remove_previous_scenes_p, bool _reset_view);

private:

    Ui::DataViewer *m_ui;

    CartoScene m_scene;

    bool m_is_toolbar_displayed;

    MatisseIconFactory *m_icon_factory;

    eCartoViewType m_current_view_type;

    QThread m_result_loading_thread;
    resultLoadingTask m_result_loading_task;

    QTimer m_3d_file_check_timer;
    QString m_3d_folder_pattern;
    QDateTime m_watcher_start_time;
    QDateTime m_last_loaded_file_time;
    bool m_watcher_first_file;

    QStringList m_supported_raster_format;
    QStringList m_supported_vector_format;
    QStringList m_supported_3d_file_format;
    QStringList m_supported_image_format;

    QAction *m_extent_auto_resize;
    QAction *m_follow_last_item;
    QAction *m_manual_move;
    QMenu *m_repaint_behavior_menu;

    eRepaintBehaviorState m_repaint_behavior_state;
    QToolBar *m_map_toolbar;
    QLabel *m_coords;

    std::vector< osg::ref_ptr<osg::Node> > m_osg_nodes;

};

} // namespace matisse

#endif // MATISSE_DATA_VIEWER_H_
