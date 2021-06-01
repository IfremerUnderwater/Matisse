#ifndef M_PI
#define M_PI 3.1415926535
#endif

#include "data_viewer.h"
#include "ui_data_viewer.h"

#include "osg_widget.h"

using namespace cv;
using namespace nav_tools;

DataViewer::DataViewer(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::DataViewer),
    _isToolBarDisplayed(false),
    _iconFactory(NULL)
{
    _ui->setupUi(this);

    // must be set to open osg files
    _resultLoadingTask.setOSGWidget(_ui->_OSG_viewer);

    // Default view is OpenSceneGraphView
    switchCartoViewTo(OpenSceneGraphView);

    _supportedRasterFormat << "tif" << "tiff";
    _supportedVectorFormat << "shp";
    _supported3DFileFormat << "obj" << "osg" << "ply" << "osgt" << "kml";
    _supportedImageFormat << "jpg" << "jpeg" << "png";

    // Init QAction & Menu
    _extentAutoResize = new QAction(tr("Auto adjust to footprint"),this);
    _extentAutoResize->setCheckable(true);
    _extentAutoResize->setChecked(true);

    _followLastItem = new QAction(tr("Follow last item"),this);
    _followLastItem->setCheckable(false);
    _followLastItem->setChecked(false);

    _manualMove = new QAction(tr("Manual move"),this);
    _manualMove->setCheckable(false);
    _manualMove->setChecked(false);

    _repaintBehaviorMenu = new QMenu(this);
    _repaintBehaviorMenu->addAction(_extentAutoResize);
    _repaintBehaviorMenu->addAction(_followLastItem);
    _repaintBehaviorMenu->addAction(_manualMove);

    _repaintBehaviorState = ExtentAutoResize;

    connect(_extentAutoResize, SIGNAL(triggered()), this, SLOT(slot_onAutoResizeTrigger()));
    connect(_followLastItem, SIGNAL(triggered()), this, SLOT(slot_onFollowLastItem()));
    connect(_manualMove, SIGNAL(triggered()), this, SLOT(slot_onManualMove()));

    // Init loading thread
    qRegisterMetaType< osg::ref_ptr<osg::Node> >();

    connect(this,SIGNAL(signal_loadRasterFromFile(QString)),&_resultLoadingTask,SLOT(slot_loadRasterFromFile(QString)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_addRasterToCartoView(CartoImage *)), this,SLOT(slot_addRasterToCartoView(CartoImage*)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_addRasterToImageView(Image *)),this,SLOT(slot_addRasterToImageView(Image *)),Qt::QueuedConnection);
    connect(this,SIGNAL(signal_load3DSceneFromFile(QString,bool)),&_resultLoadingTask,SLOT(slot_load3DSceneFromFile(QString,bool)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node>,bool)),this,SLOT(slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node>,bool)),Qt::QueuedConnection);

    _resultLoadingTask.moveToThread(&_resultLoadingThread);
    _resultLoadingThread.start();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slot_showMapContextMenu(const QPoint&)));

    _scene.setScaleFactor(4.0);
    _ui->_GV_view->setScene(&_scene);

    connect(_ui->_GV_view,SIGNAL(signal_updateCoords(QPointF)),this,SLOT(slot_updateMapCoords(QPointF)),Qt::QueuedConnection);
    connect(_ui->_GV_view,SIGNAL(signal_zoomChanged(qreal)),this,SLOT(slot_mapZoomChanged(qreal)),Qt::QueuedConnection);
    connect(_ui->_GV_view,SIGNAL(signal_panChanged()),this,SLOT(slot_mapPanChanged()),Qt::QueuedConnection);
}

DataViewer::~DataViewer()
{
    // QT 5.10 : to avoid message on exit : "QThread: Destroyed while thread is still running"
    _resultLoadingThread.exit();
    QThread::sleep(1);

//    delete _panTool;
//    delete _zoomInTool;
//    delete _zoomOutTool;
    delete _ui;
}

void DataViewer::initMapToolBar()
{
    if (!_iconFactory) {
        qCritical() << "Icon factory is not defined in UserFormWidget, cannot initialize map toolbar";
        return;
    }

    _mapToolBar = new QToolBar("Map tools", this);

       QAction *zoomInAction = new QAction(this);
       IconizedActionWrapper *zoomInActionWrapper = new IconizedActionWrapper(zoomInAction);
      _iconFactory->attachIcon(zoomInActionWrapper, "lnf/icons/carte-zoom-in.svg", false, false);
       zoomInAction->setIconText(tr("Zoom in"));
      _mapToolBar->addAction(zoomInAction);
       connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomInTool()));

      QAction *zoomOutAction = new QAction(this);
      IconizedActionWrapper *zoomOutActionWrapper = new IconizedActionWrapper(zoomOutAction);
      _iconFactory->attachIcon(zoomOutActionWrapper, "lnf/icons/carte-zoom-out.svg", false, false);
      zoomOutAction->setIconText(tr("Zoom out"));
      _mapToolBar->addAction(zoomOutAction);
      connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomOutTool()));

    QAction *recenterAction = new QAction(this);
    IconizedActionWrapper *recenterActionWrapper = new IconizedActionWrapper(recenterAction);
    _iconFactory->attachIcon(recenterActionWrapper, "lnf/icons/carte-afficher-tout.svg", false, false);
    recenterAction->setIconText(tr("Recenter"));
    _mapToolBar->addAction(recenterAction);
    connect(recenterAction, SIGNAL(triggered(bool)), this, SLOT(slot_recenterMap()));

    _mapToolBar->addSeparator();

    _coords = new QLabel("__________ __________");
    QAction *coords = _mapToolBar->addWidget(_coords);

    _mapToolBar->addSeparator();

    QAction *showImageAction = new QAction(this);
    showImageAction->setText("[]");
    showImageAction->setCheckable(true);
    showImageAction->setChecked(false);
    showImageAction->setToolTip("Bounding Image Rectangles");

    _mapToolBar->addAction(showImageAction);
    connect(showImageAction, SIGNAL(triggered(bool)), this, SLOT(slot_showImagesRect(bool)));

    _ui->_WID_pageQGis->layout()->setMenuBar(_mapToolBar);
    _mapToolBar->setVisible(false);
}

void DataViewer::slot_updateMapCoords(QPointF p)
{
    // scale needed to convert in meters (in case of GeoTiff in UTM)
    qreal scale = _scene.scale();
    QString s;
    s.sprintf("%8.1f %8.1f", p.x()/scale ,p.y()/scale);
    _coords->setText(s);
}

void  DataViewer::slot_showImagesRect(bool show)
{
    _scene.showImageRect(show);
}


void DataViewer::switchCartoViewTo(CartoViewType cartoViewType_p)
{

    this->clear();

    switch(cartoViewType_p)
    {

    case QGisMapLayer:
        _ui->_stackedWidget->setCurrentIndex(0);
        _ui->_GV_view->setZoomFactor(1.0 / _scene.scaleFactor());
        _currentViewType = QGisMapLayer;
        break;
    case QImageView:
        _ui->_stackedWidget->setCurrentIndex(1);
        _currentViewType = QImageView;
        break;
    case OpenSceneGraphView:
        _ui->_stackedWidget->setCurrentIndex(2);
        _currentViewType = OpenSceneGraphView;
        break;

    }
}


void DataViewer::initCanvas() {

    qDebug() << "Init QGIS Canvas";

//    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

//    mapCanvas->enableAntiAliasing(true);
//    //mapCanvas->useImageToRender(false);
//    /* bg color is set by signal updateColorPalette */
//    mapCanvas->freeze(false);
//    mapCanvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//    mapCanvas->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
//    mapCanvas->refresh();
//    mapCanvas->show();

}

void DataViewer::slot_updateColorPalette(QMap<QString,QString> newColorPalette)
{
    QString backgroundColor = newColorPalette.value("color.black");
    qDebug() << "Update QGIS Canvas with bg color : " << backgroundColor;
//    QgsMapCanvas* mapCanvas = _ui->_GRV_map;
//    mapCanvas->setCanvasColor(QColor(backgroundColor));
//    mapCanvas->refresh();
}

void DataViewer::slot_showHideToolbar()
{
    _isToolBarDisplayed = !_isToolBarDisplayed;
    _mapToolBar->setVisible(_isToolBarDisplayed);
}

//void UserFormWidget::slot_activatePanTool()
//{
////    _ui->_GRV_map->setMapTool(_panTool);
//}

void DataViewer::slot_activateZoomInTool()
{
//    _ui->_GRV_map->setMapTool(_zoomInTool);
    qreal zoom = _ui->_GV_view->zoomfactor();
    _ui->_GV_view->setZoomFactor(zoom*1.25);
    _ui->_GV_view->repaint();
}

void DataViewer::slot_activateZoomOutTool()
{
//    _ui->_GRV_map->setMapTool(_zoomOutTool);
    qreal zoom = _ui->_GV_view->zoomfactor();
    _ui->_GV_view->setZoomFactor(zoom*0.80);
    _ui->_GV_view->repaint();
}

void DataViewer::slot_recenterMap()
{
    // reset zoom...
    _ui->_GV_view->resetMatrix();
    _ui->_GV_view->fitInView( _scene.sceneRect(), Qt::KeepAspectRatio );
    QMatrix m = _ui->_GV_view->matrix();
    _ui->_GV_view->setZoomFactor(m.m11());
    _ui->_GV_view->repaint();
}


void DataViewer::clear()
{

    // Clear OSG Widget
    _ui->_OSG_viewer->clearSceneData();

    // clear carto view
    _scene.clearScene();
    _scene.setParentSize(_ui->_GV_view->size());
    _ui->_GV_view->resetMatrix();
}

void DataViewer::resetJobForm()
{
    // reset parameters
    clear();
}


void DataViewer::loadRasterFile(QString filename) {

    emit signal_loadRasterFromFile(filename);

}

void DataViewer::slot_addRasterToCartoView(CartoImage * image_p) {

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    displayCartoImage(image_p);
}

void DataViewer::slot_addRasterToImageView(Image * image_p)
{
    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);

    displayImage(image_p);
}

void DataViewer::load3DFile(QString filename_p, bool remove_previous_scenes_p)
{
    if (_currentViewType!=OpenSceneGraphView)
        switchCartoViewTo(OpenSceneGraphView);
    emit signal_load3DSceneFromFile(filename_p, remove_previous_scenes_p);
}

void DataViewer::slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p, bool _remove_previous_scenes)
{
    _ui->_OSG_viewer->addNodeToScene(sceneData_p);
    if(_remove_previous_scenes)
    {
        for(int i=0; i<_osg_nodes.size(); i++)
            _ui->_OSG_viewer->removeNodeFromScene(_osg_nodes[i]);
        _osg_nodes.clear();
    }
    _osg_nodes.push_back(sceneData_p);
}

void DataViewer::slot_showMapContextMenu(const QPoint &pos_p)
{
    if (_currentViewType == QGisMapLayer){
        _repaintBehaviorMenu->popup(this->mapToGlobal(pos_p));
    }
}

void DataViewer::slot_onAutoResizeTrigger()
{
    if (_repaintBehaviorState == ExtentAutoResize){
        _extentAutoResize->setChecked(true);
    }

    if (_currentViewType == QGisMapLayer){

        switch (_repaintBehaviorState) {

        case FollowLastItem:
            _repaintBehaviorState = ExtentAutoResize;
            _extentAutoResize->setCheckable(true);
            _extentAutoResize->setChecked(true);
            _followLastItem->setCheckable(false);
            break;
        case ManualMove:
            _repaintBehaviorState = ExtentAutoResize;
            _extentAutoResize->setCheckable(true);
            _extentAutoResize->setChecked(true);
            _manualMove->setCheckable(false);
            break;
        default:
            break;
        }
    }

}

void DataViewer::slot_onFollowLastItem()
{
    if (_repaintBehaviorState == FollowLastItem){
        _followLastItem->setChecked(true);
        return;
    }

    if (_currentViewType == QGisMapLayer){

        switch (_repaintBehaviorState) {

        case ExtentAutoResize:
            _repaintBehaviorState = FollowLastItem;
            _followLastItem->setCheckable(true);
            _followLastItem->setChecked(true);
            _extentAutoResize->setCheckable(false);
            break;
        case ManualMove:
            _repaintBehaviorState = FollowLastItem;
            _followLastItem->setCheckable(true);
            _followLastItem->setChecked(true);
            _manualMove->setCheckable(false);
            break;
        default:
            break;

        }
    }
}

void DataViewer::slot_onManualMove()
{
    if (_repaintBehaviorState == ManualMove){
        _manualMove->setChecked(true);
        return;
    }

    if (_currentViewType == QGisMapLayer){

        switch (_repaintBehaviorState) {

        case ExtentAutoResize:
            _repaintBehaviorState = ManualMove;
            _manualMove->setCheckable(true);
            _manualMove->setChecked(true);
            _extentAutoResize->setCheckable(false);
            break;
        case FollowLastItem:
            _repaintBehaviorState = ManualMove;
            _manualMove->setCheckable(true);
            _manualMove->setChecked(true);
            _followLastItem->setCheckable(false);
            break;
        default:
            break;

        }
    }
}


void DataViewer::exportMapViewToImage(QString imageFilePath)
{
    QImage image(QSize(800, 600), QImage::Format_ARGB32_Premultiplied);

    QColor color(0,0,0);
    image.fill(color.rgb());

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    image.save(imageFilePath, "png");
}


void DataViewer::loadImageFile(QString filename){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);

    QImage result(filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}


void DataViewer::setIconFactory(MatisseIconFactory *iconFactory)
{
    _iconFactory = iconFactory;
}


CartoViewType DataViewer::currentViewType() const
{
    return _currentViewType;
}

QStringList DataViewer::supportedRasterFormat() const
{
    return _supportedRasterFormat;
}

QStringList DataViewer::supportedVectorFormat() const
{
    return _supportedVectorFormat;
}

QStringList DataViewer::supported3DFileFormat() const
{
    return _supported3DFileFormat;
}

QStringList DataViewer::supportedImageFormat() const
{
    return _supportedImageFormat;
}

void DataViewer::displayImage(Image *image ){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);

    Mat dest;

    qDebug()<< "Channels " << image->imageData()->channels();

    cvtColor(*(image->imageData()), dest,COLOR_BGR2RGB);

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

void DataViewer::displayCartoImage(CartoImage *image ){

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    int nbobj = _scene.items().size();
    _scene.addCartoImage(image);
    if(nbobj == 0)
        slot_recenterMap();

}


void DataViewer::slot_mapZoomChanged(qreal z)
{
    if(z > _scene.scaleFactor())
    {
        _scene.setScaleFactor(_scene.scaleFactor() * 4);
    }
    _scene.reloadVisibleImageWithNewScaleFactor(_ui->_GV_view);
}

void DataViewer::slot_mapPanChanged()
{
    _scene.reloadVisibleImageWithNewScaleFactor(_ui->_GV_view);
}


// Threaded result file loading task
resultLoadingTask::resultLoadingTask()
{
}

resultLoadingTask::~resultLoadingTask()
{

}

void resultLoadingTask::slot_loadRasterFromFile(QString filename_p)
{

    if (filename_p.isEmpty()) {
        return;
    }

    CartoImage *image = new CartoImage();
    image->loadFile(filename_p);

    emit signal_addRasterToCartoView(image);

}

void resultLoadingTask::slot_load3DSceneFromFile(QString filename_p, bool remove_previous_scenes_p)
{
    // load the data
    setlocale(LC_ALL, "C");

    osg::ref_ptr<osg::Node> node = m_osgwidget->createNodeFromFile(filename_p.toStdString());

    emit signal_add3DSceneToCartoView(node, remove_previous_scenes_p);
}
