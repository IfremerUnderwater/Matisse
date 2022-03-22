#ifndef M_PI
#define M_PI 3.1415926535
#endif

#include "data_viewer.h"
#include "ui_data_viewer.h"

#include "osg_widget.h"

using namespace cv;
using namespace nav_tools;

namespace matisse {

DataViewer::DataViewer(QWidget *_parent) :
    QWidget(_parent),
    m_ui(new Ui::DataViewer),
    m_is_toolbar_displayed(false),
    m_icon_factory(NULL)
{
    m_ui->setupUi(this);

    // must be set to open osg files
    m_result_loading_task.setOSGWidget(m_ui->_OSG_viewer);

    // Default view is OpenSceneGraphView
    switchCartoViewTo(OPEN_SCENE_GRAPH_VIEW);

    m_supported_raster_format << "tif" << "tiff";
    m_supported_vector_format << "shp";
    m_supported_3d_file_format << "obj" << "osg" << "ply" << "osgt" << "kml";
    m_supported_image_format << "jpg" << "jpeg" << "png";

    // Init QAction & Menu
    m_extent_auto_resize = new QAction(tr("Auto adjust to footprint"),this);
    m_extent_auto_resize->setCheckable(true);
    m_extent_auto_resize->setChecked(true);

    m_follow_last_item = new QAction(tr("Follow last item"),this);
    m_follow_last_item->setCheckable(false);
    m_follow_last_item->setChecked(false);

    m_manual_move = new QAction(tr("Manual move"),this);
    m_manual_move->setCheckable(false);
    m_manual_move->setChecked(false);

    m_repaint_behavior_menu = new QMenu(this);
    m_repaint_behavior_menu->addAction(m_extent_auto_resize);
    m_repaint_behavior_menu->addAction(m_follow_last_item);
    m_repaint_behavior_menu->addAction(m_manual_move);

    m_repaint_behavior_state = EXTENT_AUTO_RESIZE;

    connect(&m_3d_file_check_timer, SIGNAL(timeout()), this, SLOT(sl_checkFor3DFiles()));

    connect(m_extent_auto_resize, SIGNAL(triggered()), this, SLOT(sl_onAutoResizeTrigger()));
    connect(m_follow_last_item, SIGNAL(triggered()), this, SLOT(sl_onFollowLastItem()));
    connect(m_manual_move, SIGNAL(triggered()), this, SLOT(sl_onManualMove()));

    // Init loading thread
    qRegisterMetaType< osg::ref_ptr<osg::Node> >();

    connect(this,SIGNAL(si_loadRasterFromFile(QString)),&m_result_loading_task,SLOT(sl_loadRasterFromFile(QString)),Qt::QueuedConnection);
    connect(&m_result_loading_task,SIGNAL(si_addRasterToCartoView(CartoImage *)), this,SLOT(sl_addRasterToCartoView(CartoImage*)),Qt::QueuedConnection);
    connect(&m_result_loading_task,SIGNAL(si_addRasterToImageView(Image *)),this,SLOT(sl_addRasterToImageView(Image *)),Qt::QueuedConnection);
    connect(this,SIGNAL(si_load3DSceneFromFile(QString,bool, bool)),&m_result_loading_task,SLOT(sl_load3DSceneFromFile(QString,bool,bool)),Qt::QueuedConnection);
    connect(&m_result_loading_task,SIGNAL(si_add3DSceneToCartoView(osg::ref_ptr<osg::Node>,bool, bool)),this,SLOT(sl_add3DSceneToCartoView(osg::ref_ptr<osg::Node>,bool,bool)),Qt::QueuedConnection);

    m_result_loading_task.moveToThread(&m_result_loading_thread);
    m_result_loading_thread.start();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(sl_showMapContextMenu(const QPoint&)));

    m_scene.setScaleFactor(4.0);
    m_ui->_GV_view->setScene(&m_scene);

    connect(m_ui->_GV_view,SIGNAL(si_updateCoords(QPointF)),this,SLOT(sl_updateMapCoords(QPointF)),Qt::QueuedConnection);
    connect(m_ui->_GV_view,SIGNAL(si_zoomChanged(qreal)),this,SLOT(sl_mapZoomChanged(qreal)),Qt::QueuedConnection);
    connect(m_ui->_GV_view,SIGNAL(si_panChanged()),this,SLOT(sl_mapPanChanged()),Qt::QueuedConnection);
}

DataViewer::~DataViewer()
{
    // QT 5.10 : to avoid message on exit : "QThread: Destroyed while thread is still running"
    m_result_loading_thread.exit();
    QThread::sleep(1);

    delete m_ui;
}

void DataViewer::initMapToolBar()
{
    if (!m_icon_factory) {
        qCritical() << "Icon factory is not defined in UserFormWidget, cannot initialize map toolbar";
        return;
    }

    m_map_toolbar = new QToolBar("Map tools", this);

       QAction *zoom_in_action = new QAction(this);
       IconizedActionWrapper *zoom_in_action_wrapper = new IconizedActionWrapper(zoom_in_action);
      m_icon_factory->attachIcon(zoom_in_action_wrapper, "lnf/icons/carte-zoom-in.svg", false, false);
       zoom_in_action->setIconText(tr("Zoom in"));
      m_map_toolbar->addAction(zoom_in_action);
       connect(zoom_in_action, SIGNAL(triggered(bool)), this, SLOT(sl_activateZoomInTool()));

      QAction *zoom_out_action = new QAction(this);
      IconizedActionWrapper *zoom_out_action_wrapper = new IconizedActionWrapper(zoom_out_action);
      m_icon_factory->attachIcon(zoom_out_action_wrapper, "lnf/icons/carte-zoom-out.svg", false, false);
      zoom_out_action->setIconText(tr("Zoom out"));
      m_map_toolbar->addAction(zoom_out_action);
      connect(zoom_out_action, SIGNAL(triggered(bool)), this, SLOT(sl_activateZoomOutTool()));

    QAction *recenter_action = new QAction(this);
    IconizedActionWrapper *recenter_action_wrapper = new IconizedActionWrapper(recenter_action);
    m_icon_factory->attachIcon(recenter_action_wrapper, "lnf/icons/carte-afficher-tout.svg", false, false);
    recenter_action->setIconText(tr("Recenter"));
    m_map_toolbar->addAction(recenter_action);
    connect(recenter_action, SIGNAL(triggered(bool)), this, SLOT(sl_recenterMap()));

    m_map_toolbar->addSeparator();

    m_coords = new QLabel("__________ __________");
    QAction *coords = m_map_toolbar->addWidget(m_coords);

    m_map_toolbar->addSeparator();

    QAction *show_image_action = new QAction(this);
    show_image_action->setText("[]");
    show_image_action->setCheckable(true);
    show_image_action->setChecked(false);
    show_image_action->setToolTip("Bounding Image Rectangles");

    m_map_toolbar->addAction(show_image_action);
    connect(show_image_action, SIGNAL(triggered(bool)), this, SLOT(sl_showImagesRect(bool)));

    m_ui->_WID_pageQGis->layout()->setMenuBar(m_map_toolbar);
    m_map_toolbar->setVisible(false);
}

void DataViewer::sl_updateMapCoords(QPointF _p)
{
    // scale needed to convert in meters (in case of GeoTiff in UTM)
    qreal scale = m_scene.scale();
    QString s;
    s.sprintf("%8.1f %8.1f", _p.x()/scale ,_p.y()/scale);
    m_coords->setText(s);
}

void  DataViewer::sl_showImagesRect(bool show)
{
    m_scene.showImageRect(show);
}


void DataViewer::switchCartoViewTo(eCartoViewType _carto_view_type_p)
{

    this->clear();

    switch(_carto_view_type_p)
    {

    case QGIS_MAP_LAYER:
        m_ui->_stackedWidget->setCurrentIndex(0);
        m_ui->_GV_view->setZoomFactor(1.0 / m_scene.scaleFactor());
        m_current_view_type = QGIS_MAP_LAYER;
        break;
    case QIMAGE_VIEW:
        m_ui->_stackedWidget->setCurrentIndex(1);
        m_current_view_type = QIMAGE_VIEW;
        break;
    case OPEN_SCENE_GRAPH_VIEW:
        m_ui->_stackedWidget->setCurrentIndex(2);
        m_current_view_type = OPEN_SCENE_GRAPH_VIEW;
        break;

    }
}


void DataViewer::initCanvas() {

    qDebug() << "Init QGIS Canvas";

    /* QGis option deprecated */
}

void DataViewer::sl_updateColorPalette(QMap<QString,QString> newColorPalette)
{
    QString background_color = newColorPalette.value("color.black");
    qDebug() << "Update QGIS Canvas with bg color : " << background_color;
    /* QGis option deprecated */
}

void DataViewer::sl_showHideToolbar()
{
    m_is_toolbar_displayed = !m_is_toolbar_displayed;
    m_map_toolbar->setVisible(m_is_toolbar_displayed);
}


void DataViewer::sl_activateZoomInTool()
{
//    _ui->_GRV_map->setMapTool(_zoomInTool);
    qreal zoom = m_ui->_GV_view->zoomfactor();
    m_ui->_GV_view->setZoomFactor(zoom*1.25);
    m_ui->_GV_view->repaint();
}

void DataViewer::sl_activateZoomOutTool()
{
    qreal zoom = m_ui->_GV_view->zoomfactor();
    m_ui->_GV_view->setZoomFactor(zoom*0.80);
    m_ui->_GV_view->repaint();
}

void DataViewer::sl_recenterMap()
{
    // reset zoom...
    m_ui->_GV_view->resetMatrix();
    m_ui->_GV_view->fitInView( m_scene.sceneRect(), Qt::KeepAspectRatio );
    QMatrix m = m_ui->_GV_view->matrix();
    m_ui->_GV_view->setZoomFactor(m.m11());
    m_ui->_GV_view->repaint();
}


void DataViewer::clear()
{

    // Clear OSG Widget
    m_ui->_OSG_viewer->clearSceneData();

    // clear carto view
    m_scene.clearScene();
    m_scene.setParentSize(m_ui->_GV_view->size());
    m_ui->_GV_view->resetMatrix();
}

void DataViewer::resetJobForm()
{
    // reset parameters
    clear();
}


void DataViewer::loadRasterFile(QString _filename) {

    emit si_loadRasterFromFile(_filename);
}

void DataViewer::sl_addRasterToCartoView(CartoImage * _image_p) {

    if (m_current_view_type!=QGIS_MAP_LAYER)
        switchCartoViewTo(QGIS_MAP_LAYER);

    displayCartoImage(_image_p);
}

void DataViewer::sl_addRasterToImageView(Image * _image_p)
{
    if (m_current_view_type!=QIMAGE_VIEW)
        switchCartoViewTo(QIMAGE_VIEW);

    displayImage(_image_p);
}

void DataViewer::load3DFile(QString _filename_p, bool _remove_previous_scenes_p, bool _reset_view)
{
    if (m_current_view_type!=OPEN_SCENE_GRAPH_VIEW)
        switchCartoViewTo(OPEN_SCENE_GRAPH_VIEW);
    emit si_load3DSceneFromFile(_filename_p, _remove_previous_scenes_p, _reset_view);
}

void DataViewer::autoAdd3DFileFromFolderOnMainView(QString _folderpath_p)
{
    m_watcher_first_file = true;

    if (!_folderpath_p.isEmpty())
    {
        m_3d_folder_pattern = _folderpath_p;
        m_3d_file_check_timer.start(1000);
        m_watcher_start_time = QDateTime::currentDateTime();
    }
    else
    {
        m_3d_file_check_timer.stop();
        m_3d_folder_pattern = "";
    }
        
}

void DataViewer::sl_add3DSceneToCartoView(osg::ref_ptr<osg::Node> _scene_data_p, bool _remove_previous_scenes, bool _reset_view)
{
    m_ui->_OSG_viewer->addNodeToScene(_scene_data_p, 0.0, _reset_view);
    if(_remove_previous_scenes)
    {
        for(int i=0; i<m_osg_nodes.size(); i++)
            m_ui->_OSG_viewer->removeNodeFromScene(m_osg_nodes[i]);
        m_osg_nodes.clear();
    }
    m_osg_nodes.push_back(_scene_data_p);
}

void DataViewer::sl_checkFor3DFiles()
{
    QFileInfo three_d_folder(m_3d_folder_pattern);
    QDir export_folder(three_d_folder.absolutePath());
    export_folder.setNameFilters(QStringList() << three_d_folder.fileName());
    QStringList file_list = export_folder.entryList();

    bool can_show_a_model = false;
    QString model_path;

    foreach(QString ply_file, file_list)
    {
        QFileInfo ply_file_info(three_d_folder.absolutePath() + QDir::separator() + ply_file);
        QDateTime ply_last_mod = ply_file_info.lastModified();
        if (ply_last_mod > m_watcher_start_time && ply_last_mod > m_last_loaded_file_time)
        {
            model_path = ply_file_info.absoluteFilePath();
            m_last_loaded_file_time = ply_last_mod;
            can_show_a_model = true;
        }
    }

    if (can_show_a_model)
    {
        if(m_watcher_first_file)
            this->load3DFile(model_path, true, true);
        else
            this->load3DFile(model_path, true, false);

        m_watcher_first_file = false;
    }
        
}

void DataViewer::sl_showMapContextMenu(const QPoint &_pos_p)
{
    if (m_current_view_type == QGIS_MAP_LAYER){
        m_repaint_behavior_menu->popup(this->mapToGlobal(_pos_p));
    }
}

void DataViewer::sl_onAutoResizeTrigger()
{
    if (m_repaint_behavior_state == EXTENT_AUTO_RESIZE){
        m_extent_auto_resize->setChecked(true);
    }

    if (m_current_view_type == QGIS_MAP_LAYER){

        switch (m_repaint_behavior_state) {

        case FOLLOW_LAST_ITEM:
            m_repaint_behavior_state = EXTENT_AUTO_RESIZE;
            m_extent_auto_resize->setCheckable(true);
            m_extent_auto_resize->setChecked(true);
            m_follow_last_item->setCheckable(false);
            break;
        case MANUAL_MOVE:
            m_repaint_behavior_state = EXTENT_AUTO_RESIZE;
            m_extent_auto_resize->setCheckable(true);
            m_extent_auto_resize->setChecked(true);
            m_manual_move->setCheckable(false);
            break;
        default:
            break;
        }
    }

}

void DataViewer::sl_onFollowLastItem()
{
    if (m_repaint_behavior_state == FOLLOW_LAST_ITEM){
        m_follow_last_item->setChecked(true);
        return;
    }

    if (m_current_view_type == QGIS_MAP_LAYER){

        switch (m_repaint_behavior_state) {

        case EXTENT_AUTO_RESIZE:
            m_repaint_behavior_state = FOLLOW_LAST_ITEM;
            m_follow_last_item->setCheckable(true);
            m_follow_last_item->setChecked(true);
            m_extent_auto_resize->setCheckable(false);
            break;
        case MANUAL_MOVE:
            m_repaint_behavior_state = FOLLOW_LAST_ITEM;
            m_follow_last_item->setCheckable(true);
            m_follow_last_item->setChecked(true);
            m_manual_move->setCheckable(false);
            break;
        default:
            break;

        }
    }
}

void DataViewer::sl_onManualMove()
{
    if (m_repaint_behavior_state == MANUAL_MOVE){
        m_manual_move->setChecked(true);
        return;
    }

    if (m_current_view_type == QGIS_MAP_LAYER){

        switch (m_repaint_behavior_state) {

        case EXTENT_AUTO_RESIZE:
            m_repaint_behavior_state = MANUAL_MOVE;
            m_manual_move->setCheckable(true);
            m_manual_move->setChecked(true);
            m_extent_auto_resize->setCheckable(false);
            break;
        case FOLLOW_LAST_ITEM:
            m_repaint_behavior_state = MANUAL_MOVE;
            m_manual_move->setCheckable(true);
            m_manual_move->setChecked(true);
            m_follow_last_item->setCheckable(false);
            break;
        default:
            break;

        }
    }
}


void DataViewer::exportMapViewToImage(QString _image_file_path)
{
    QImage image(QSize(800, 600), QImage::Format_ARGB32_Premultiplied);

    QColor color(0,0,0);
    image.fill(color.rgb());

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    image.save(_image_file_path, "png");
}


void DataViewer::loadImageFile(QString _filename){

    if (m_current_view_type!=QIMAGE_VIEW)
        switchCartoViewTo(QIMAGE_VIEW);

    QImage result(_filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = m_ui->_LA_resultImage->size();
    this->m_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}


void DataViewer::setIconFactory(MatisseIconFactory *_icon_factory)
{
    m_icon_factory = _icon_factory;
}


eCartoViewType DataViewer::currentViewType() const
{
    return m_current_view_type;
}

QStringList DataViewer::supportedRasterFormat() const
{
    return m_supported_raster_format;
}

QStringList DataViewer::supportedVectorFormat() const
{
    return m_supported_vector_format;
}

QStringList DataViewer::supported3DFileFormat() const
{
    return m_supported_3d_file_format;
}

QStringList DataViewer::supportedImageFormat() const
{
    return m_supported_image_format;
}

void DataViewer::displayImage(Image *_image ){

    if (m_current_view_type!=QIMAGE_VIEW)
        switchCartoViewTo(QIMAGE_VIEW);

    Mat dest;

    qDebug()<< "Channels " << _image->imageData()->channels();

    cvtColor(*(_image->imageData()), dest,COLOR_BGR2RGB);

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = m_ui->_LA_resultImage->size();
    this->m_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

void DataViewer::displayCartoImage(CartoImage *_image ){

    if (m_current_view_type!=QGIS_MAP_LAYER)
        switchCartoViewTo(QGIS_MAP_LAYER);

    int nbobj = m_scene.items().size();
    m_scene.addCartoImage(_image);
    if(nbobj == 0)
        sl_recenterMap();

}


void DataViewer::sl_mapZoomChanged(qreal _z)
{
    if(_z > m_scene.scaleFactor())
    {
        m_scene.setScaleFactor(m_scene.scaleFactor() * 4);
    }
    m_scene.reloadVisibleImageWithNewScaleFactor(m_ui->_GV_view);
}

void DataViewer::sl_mapPanChanged()
{
    m_scene.reloadVisibleImageWithNewScaleFactor(m_ui->_GV_view);
}


// Threaded result file loading task
resultLoadingTask::resultLoadingTask()
{
}

resultLoadingTask::~resultLoadingTask()
{

}

void resultLoadingTask::sl_loadRasterFromFile(QString _filename_p)
{

    if (_filename_p.isEmpty()) {
        return;
    }

    CartoImage *image = new CartoImage();
    image->loadFile(_filename_p);

    emit si_addRasterToCartoView(image);

}

void resultLoadingTask::sl_load3DSceneFromFile(QString _filename_p, bool _remove_previous_scenes_p, bool _reset_view)
{
    // load the data
    setlocale(LC_ALL, "C");

    osg::ref_ptr<osg::Node> node = m_osgwidget->createNodeFromFile(_filename_p.toStdString());

    emit si_add3DSceneToCartoView(node, _remove_previous_scenes_p, _reset_view);
}

} // namespace matisse
