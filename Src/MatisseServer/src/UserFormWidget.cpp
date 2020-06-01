#define M_PI 3.1415926535
#include "UserFormWidget.h"
#include "ui_UserFormWidget.h"

#include "WheelGraphicsView.h"

#include "OSGWidget/osg_widget.h"

// Qgis 2.99
//#include <qgssymbol.h>
//#include <qgssinglesymbolrenderer.h>
//#include <qgsmaprendererjob.h>
//#include <qgsmaprenderersequentialjob.h>


using namespace cv;


UserFormWidget::UserFormWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::UserFormWidget),
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
    connect(this,SIGNAL(signal_load3DSceneFromFile(QString)),&_resultLoadingTask,SLOT(slot_load3DSceneFromFile(QString)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),this,SLOT(slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),Qt::QueuedConnection);

    _resultLoadingTask.moveToThread(&_resultLoadingThread);
    _resultLoadingThread.start();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slot_showMapContextMenu(const QPoint&)));

    // WheelGraphicsView replace standard GraphicsView
//    _GV_view = new WheelGraphicsView(_ui->_WID_pageQGis);
//    _GV_view->resize(_ui->_WID_pageQGis->width(), _ui->_WID_pageQGis->height());

    // handled by WheelGraphicsView
    //_ui->_GV_view->setDragMode(QGraphicsView::ScrollHandDrag);
    _scene.setScaleFactor(4.0);
    _ui->_GV_view->setScene(&_scene);

    connect(_ui->_GV_view,SIGNAL(signal_updateCoords(QPointF)),this,SLOT(slot_updateMapCoords(QPointF)),Qt::QueuedConnection);
    connect(_ui->_GV_view,SIGNAL(signal_zoomChanged(qreal)),this,SLOT(slot_mapZoomChanged(qreal)),Qt::QueuedConnection);
    connect(_ui->_GV_view,SIGNAL(signal_panChanged()),this,SLOT(slot_mapPanChanged()),Qt::QueuedConnection);
}

UserFormWidget::~UserFormWidget()
{
    // QT 5.10 : to avoid message on exit : "QThread: Destroyed while thread is still running"
    _resultLoadingThread.exit();
    QThread::sleep(1);

//    delete _panTool;
//    delete _zoomInTool;
//    delete _zoomOutTool;
    delete _ui;
}

void UserFormWidget::initMapToolBar()
{
    if (!_iconFactory) {
        qCritical() << "Icon factory is not defined in UserFormWidget, cannot initialize map toolbar";
        return;
    }

    _mapToolBar = new QToolBar("Map tools", this);
//    QAction *panAction = new QAction(this);
//    IconizedActionWrapper *panActionWrapper = new IconizedActionWrapper(panAction);
//    _iconFactory->attachIcon(panActionWrapper, "lnf/icons/carte-deplacer.svg", false, false);
//    panAction->setIconText(tr("Deplacer"));
//    _mapToolBar->addAction(panAction);
//    connect(panAction, SIGNAL(triggered(bool)), this, SLOT(slot_activatePanTool()));
//    _panTool = new QgsMapToolPan(_ui->_GRV_map);
//    _panTool->setAction(panAction);

//    QAction *zoomInAction = new QAction(this);
//    IconizedActionWrapper *zoomInActionWrapper = new IconizedActionWrapper(zoomInAction);
//    _iconFactory->attachIcon(zoomInActionWrapper, "lnf/icons/carte-zoom-in.svg", false, false);
//    zoomInAction->setIconText(tr("Zoom AV"));
//    _mapToolBar->addAction(zoomInAction);
//    connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomInTool()));
//    _zoomInTool = new QgsMapToolZoom(_ui->_GRV_map, false);
//    _zoomInTool->setAction(zoomInAction);

       QAction *zoomInAction = new QAction(this);
       IconizedActionWrapper *zoomInActionWrapper = new IconizedActionWrapper(zoomInAction);
      _iconFactory->attachIcon(zoomInActionWrapper, "lnf/icons/carte-zoom-in.svg", false, false);
       zoomInAction->setIconText(tr("Zoom in"));
      _mapToolBar->addAction(zoomInAction);
       connect(zoomInAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomInTool()));

//    QAction *zoomOutAction = new QAction(this);
//    IconizedActionWrapper *zoomOutActionWrapper = new IconizedActionWrapper(zoomOutAction);
//    _iconFactory->attachIcon(zoomOutActionWrapper, "lnf/icons/carte-zoom-out.svg", false, false);
//    zoomOutAction->setIconText(tr("Zoom AR"));
//    _mapToolBar->addAction(zoomOutAction);
//    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomOutTool()));
//    _zoomOutTool = new QgsMapToolZoom(_ui->_GRV_map, true);
//    _zoomOutTool->setAction(zoomOutAction);

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

//    _mapToolBar->setFloatable(true);
//    _mapToolBar->setMovable(true);
//    _mapToolBar->setOrientation(Qt::Vertical);
//    _mapToolBar->show();

    _mapToolBar->addSeparator();

    _coords = new QLabel("__________ __________");
    QAction *coords = _mapToolBar->addWidget(_coords);

    _mapToolBar->addSeparator();

    QAction *showImageAction = new QAction(this);
    showImageAction->setText("[]");
    showImageAction->setCheckable(true);
    showImageAction->setChecked(false);
    showImageAction->setToolTip("Bounding Image Rectangles");
//    IconizedActionWrapper *recenterActionWrapper = new IconizedActionWrapper(recenterAction);
//    _iconFactory->attachIcon(recenterActionWrapper, "lnf/icons/carte-afficher-tout.svg", false, false);
//    recenterAction->setIconText(tr("Recentrer"));
    _mapToolBar->addAction(showImageAction);
    connect(showImageAction, SIGNAL(triggered(bool)), this, SLOT(slot_showImagesRect(bool)));

    _ui->_WID_pageQGis->layout()->setMenuBar(_mapToolBar);
    _mapToolBar->setVisible(false);
}

void UserFormWidget::slot_updateMapCoords(QPointF p)
{
    // scale needed to convert in meters (in case of GeoTiff in UTM)
    qreal scale = _scene.scale();
    QString s;
    s.sprintf("%8.1f %8.1f", p.x()/scale ,p.y()/scale);
    _coords->setText(s);
}

void  UserFormWidget::slot_showImagesRect(bool show)
{
    _scene.showImageRect(show);
}

void UserFormWidget::initLayersWidget()
{
    _layersWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _layersWidget->setDragEnabled(true);
    _layersWidget->viewport()->setAcceptDrops(true);
    _layersWidget->setDefaultDropAction(Qt::MoveAction);
    _layersWidget->setDropIndicatorShown(true);
    _layersWidget->setDragDropMode(QAbstractItemView::InternalMove);

    // QGIS 2.99
    //connect(QgsMapLayerRegistry::instance(), SIGNAL(layerWasAdded(QgsMapLayer*)), this, SLOT(slot_layerWasAdded(QgsMapLayer*)));
    //connect(QgsMapLayerRegistry::instance(), SIGNAL(layerRemoved(QString)), this, SLOT(slot_layerWasRemoved(QString)));
//    connect(QgsProject::instance(), SIGNAL(layerWasAdded(QgsMapLayer*)), this, SLOT(slot_layerWasAdded(QgsMapLayer*)));
//    connect(QgsProject::instance(), SIGNAL(layerRemoved(QString)), this, SLOT(slot_layerWasRemoved(QString)));
    connect(_layersWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(slot_layerItemChanged()));
    connect(_layersWidget->model(), SIGNAL(layoutChanged()), this, SLOT(slot_layerItemChanged()));

    /* Initialize context menu */
    _removeLayerAction = new QAction(tr("Supprimer"), this);
    _layersMenu = new QMenu(this);
    _layersMenu->addAction(_removeLayerAction);

    _layersWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_layersWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slot_showLayersWidgetContextMenu(QPoint)));
    connect(_removeLayerAction, SIGNAL(triggered()), this, SLOT(slot_removeLayer()));
}

void UserFormWidget::switchCartoViewTo(CartoViewType cartoViewType_p)
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


void UserFormWidget::initCanvas() {

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

void UserFormWidget::slot_updateColorPalette(QMap<QString,QString> newColorPalette)
{
    QString backgroundColor = newColorPalette.value("color.black");
    qDebug() << "Update QGIS Canvas with bg color : " << backgroundColor;
//    QgsMapCanvas* mapCanvas = _ui->_GRV_map;
//    mapCanvas->setCanvasColor(QColor(backgroundColor));
//    mapCanvas->refresh();
}

void UserFormWidget::slot_showHideToolbar()
{
    _isToolBarDisplayed = !_isToolBarDisplayed;
    _mapToolBar->setVisible(_isToolBarDisplayed);
}

//void UserFormWidget::slot_activatePanTool()
//{
////    _ui->_GRV_map->setMapTool(_panTool);
//}

void UserFormWidget::slot_activateZoomInTool()
{
//    _ui->_GRV_map->setMapTool(_zoomInTool);
    qreal zoom = _ui->_GV_view->zoomfactor();
    _ui->_GV_view->setZoomFactor(zoom*1.25);
    _ui->_GV_view->repaint();
}

void UserFormWidget::slot_activateZoomOutTool()
{
//    _ui->_GRV_map->setMapTool(_zoomOutTool);
    qreal zoom = _ui->_GV_view->zoomfactor();
    _ui->_GV_view->setZoomFactor(zoom*0.80);
    _ui->_GV_view->repaint();
}

void UserFormWidget::slot_recenterMap()
{
    // reset zoom...
    _ui->_GV_view->resetMatrix();
    _ui->_GV_view->fitInView( _scene.sceneRect(), Qt::KeepAspectRatio );
    QMatrix m = _ui->_GV_view->matrix();
    _ui->_GV_view->setZoomFactor(m.m11());
    _ui->_GV_view->repaint();
}

//void UserFormWidget::slot_layerWasAdded(QgsMapLayer *layer)
//{
//    QListWidgetItem *layerItem = new QListWidgetItem(layer->id());
//    layerItem->setCheckState(Qt::Checked);
//    layerItem->setToolTip(layer->name());
//    _layersWidget->addItem(layerItem);
//}

//void UserFormWidget::slot_layerWasRemoved(QString layerId)
//{
//    QList<QListWidgetItem *> foundItems = _layersWidget->findItems(layerId, Qt::MatchExactly);

//    if (foundItems.isEmpty()) {
//        qWarning() << QString("Layer '%1' was not found in layers widget").arg(layerId);
//        return;
//    }

//    if (foundItems.size() > 1) {
//        qWarning() << QString("Found %1 layers with id '%2', they will all be removed").arg(foundItems.size()).arg(layerId);
//    } else {
//        qDebug() << QString("Removing layer '%1' from layers widget").arg(layerId);
//    }

//    foreach (QListWidgetItem *item, foundItems) {
//        _layersWidget->removeItemWidget(item);
//        delete item;
//    }
//}

void UserFormWidget::slot_layerItemChanged()
{
//    /* rebuild layers list from updated graphical list */

//    // QGis 2.99
//    //QMap<QString, QgsMapLayer*> layersById = QgsMapLayerRegistry::instance()->mapLayers();
//    QMap<QString, QgsMapLayer*> layersById = QgsProject::instance()->mapLayers();

//    // QGis 2.99
//    //_layers.clear();
//    _players.clear();   // TODO ne doit-on pas faire de delete ?

//    for (int row = 0; row < _layersWidget->count() ; row++) {
//        QListWidgetItem *currentItem = _layersWidget->item(row);
//        QString currentLayerId = currentItem->text();
//        QgsMapLayer *layer = layersById.value(currentLayerId);
//        // QGis 2.99
//        //QgsMapCanvasLayer canvasLayer(layer);
//        QgsMapLayer::LayerType ltype = layer->type();
//        switch(ltype)
//        {
//        case QgsMapLayer::VectorLayer:
//        case QgsMapLayer::RasterLayer:
//        case QgsMapLayer::PluginLayer:
//            break;
//        }
//        bool isVisible = (currentItem->checkState() == Qt::Checked);

//        qDebug() << QString("Layer %1 has visibility %2").arg(currentLayerId).arg(isVisible);
//        // QGis 2.99
//        //canvasLayer.setVisible(isVisible);
//        // QGis 2.99
//        //_layers.append(canvasLayer);
//        _players.append(layer);
//    }

//   // QGis 2.99
//    //_ui->_GRV_map->setLayerSet(_layers);
//    _ui->_GRV_map->setLayers(_players);
}

void UserFormWidget::slot_removeLayer()
{
    QListWidgetItem *currentLayerItem = _layersWidget->currentItem();
    if (!currentLayerItem) {
        qCritical() << QString("Current layer item not identified, cannot remove layer");
        return;
    }

    QString currentLayerId = currentLayerItem->text();

    qDebug() << QString("Removing layer id '%1'...").arg(currentLayerId);

    bool foundCanvasLayer = false;

    // QGis 2.99
//    for (int i=0 ; i < _layers.size() ; i++) {
//        QgsMapCanvasLayer canvasLayer = _layers.at(i);
//        if (canvasLayer.layer()->id() == currentLayerId) {
//            _layers.removeAt(i);
//            foundCanvasLayer = true;
//        }
//    }
//    for (int i=0 ; i < _players.size() ; i++) {
//        QgsMapLayer *canvasLayer = _players.at(i);
//        if (canvasLayer->id() == currentLayerId) {
//            _players.removeAt(i);
//            foundCanvasLayer = true;
//        }
//    }

    if (!foundCanvasLayer) {
        qCritical() << QString("Canvas layer not found for id '%1'").arg(currentLayerId);
        return;
    }

    // QGis 2.99
    //QgsMapLayer *layer = QgsMapLayerRegistry::instance()->mapLayer(currentLayerId);
//    QgsMapLayer *layer = QgsProject::instance()->mapLayer(currentLayerId);

//    if (!layer) {
//        qCritical() << QString("Layer id '%1' not referenced in layer registry, cannot remove layer properly").arg(currentLayerId);
//        return;
//    }

    // QGis 2.99
    //QgsMapLayerRegistry::instance()->removeMapLayer(currentLayerId); /* layer object deleted here */
//    QgsProject::instance()->removeMapLayer(currentLayerId); /* layer object deleted here */
    /* Layer item removal is handled by signal/slot from here */

    /* recenter dans reload layers */
//    updateMapCanvasAndExtent(NULL);
}

void UserFormWidget::clear()
{
//    // Clear QGis Widget
//    // QGis 2.99
//    //QgsMapLayerRegistry::instance()->removeAllMapLayers();
//    //_layers.clear();
//    QgsProject::instance()->removeAllMapLayers();
//    _players.clear();

//    _ui->_GRV_map->clearExtentHistory();
//    //_ui->_GRV_map->clear();
//    _ui->_GRV_map->refresh();

    // Clear OSG Widget
    _ui->_OSG_viewer->clearSceneData();

    // clear carto view
    _scene.clearScene();
    _scene.setParentSize(_ui->_GV_view->size());
    _ui->_GV_view->resetMatrix();
}

void UserFormWidget::resetJobForm()
{
    // reset parameters
    qDebug() << "resetJobForm";
    clear();
}


void UserFormWidget::loadRasterFile(QString filename) {

    emit signal_loadRasterFromFile(filename);

}

//void UserFormWidget::slot_addRasterToCartoView(QgsRasterLayer * rasterLayer_p) {

//    if (_currentViewType!=QGisMapLayer)
//        switchCartoViewTo(QGisMapLayer);

////    // Add the raster Layer to the Layer Registry
////    // QGis 2.99
////    //QgsMapLayerRegistry::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);
////    QgsProject::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);

////    // Add the layer to the Layer Set
////    // QGis 2.99
////    //_layers.append(QgsMapCanvasLayer(rasterLayer_p, TRUE));//bool visibility
////    _players.append(rasterLayer_p);//bool visibility

////    this->updateMapCanvasAndExtent(NULL);

//}

void UserFormWidget::slot_addRasterToCartoView(CartoImage * image_p) {

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    displayCartoImage(image_p);
}

void UserFormWidget::slot_addRasterToImageView(Image * image_p)
{
    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);

    displayImage(image_p);
}


void UserFormWidget::slot_showLayersWidgetContextMenu(const QPoint &pos)
{
    QListWidgetItem *selectedItem = _layersWidget->currentItem();
    if (!selectedItem) {
        qDebug() << "No layer item selected, context menu not shown";
        return;
    }

    _layersMenu->popup(_layersWidget->viewport()->mapToGlobal(pos));
}

void UserFormWidget::loadShapefile(QString filename)
{
    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

//    QGis dep removed
}

void UserFormWidget::load3DFile(QString filename_p)
{
    if (_currentViewType!=OpenSceneGraphView)
        switchCartoViewTo(OpenSceneGraphView);
    emit signal_load3DSceneFromFile(filename_p);
}

void UserFormWidget::slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p)
{
    _ui->_OSG_viewer->addNodeToScene(sceneData_p);
}

void UserFormWidget::slot_showMapContextMenu(const QPoint &pos_p)
{
    if (_currentViewType == QGisMapLayer){
        _repaintBehaviorMenu->popup(this->mapToGlobal(pos_p));
    }
}

void UserFormWidget::slot_onAutoResizeTrigger()
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

void UserFormWidget::slot_onFollowLastItem()
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

void UserFormWidget::slot_onManualMove()
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



void UserFormWidget::saveQgisProject(QString filename)
{
//    QGis dep removed
}

void UserFormWidget::addPolygonToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

}

void UserFormWidget::addPolylineToMap(basicproc::Polygon &polygon_p, QString polyColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

}

void UserFormWidget::exportMapViewToImage(QString imageFilePath)
{
    QImage image(QSize(800, 600), QImage::Format_ARGB32_Premultiplied);

    QColor color(0,0,0);
    image.fill(color.rgb());

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    image.save(imageFilePath, "png");
}


void UserFormWidget::loadImageFile(QString filename){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);

    QImage result(filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}


bool UserFormWidget::findLayerIndexFromName(const QString &layerName_p, int &idx_p)
{
    int i=0;

    // Qgis 2.99
//    foreach (QgsMapCanvasLayer currentLayer, _layers){
//        if(currentLayer.layer()->name() == layerName_p){
//    foreach (QgsMapLayer *currentLayer, _players){
//        if(currentLayer->name() == layerName_p){
//            idx_p = i;
//            return true;
//        }

//        i++;
//    }

    return false;

}
void UserFormWidget::setLayersWidget(QListWidget *layersWidget)
{
    _layersWidget = layersWidget;
}

void UserFormWidget::setIconFactory(MatisseIconFactory *iconFactory)
{
    _iconFactory = iconFactory;
}


CartoViewType UserFormWidget::currentViewType() const
{
    return _currentViewType;
}

QStringList UserFormWidget::supportedRasterFormat() const
{
    return _supportedRasterFormat;
}

QStringList UserFormWidget::supportedVectorFormat() const
{
    return _supportedVectorFormat;
}

QStringList UserFormWidget::supported3DFileFormat() const
{
    return _supported3DFileFormat;
}

QStringList UserFormWidget::supportedImageFormat() const
{
    return _supportedImageFormat;
}

void UserFormWidget::displayImage(Image *image ){

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

void UserFormWidget::displayCartoImage(CartoImage *image ){

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    int nbobj = _scene.items().size();
    _scene.addCartoImage(image);
    if(nbobj == 0)
        slot_recenterMap();

}


void UserFormWidget::slot_mapZoomChanged(qreal z)
{
    if(z > _scene.scaleFactor())
    {
        _scene.setScaleFactor(_scene.scaleFactor() * 4);
    }
    _scene.reloadVisibleImageWithNewScaleFactor(_ui->_GV_view);
}

void UserFormWidget::slot_mapPanChanged()
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

void resultLoadingTask::slot_load3DSceneFromFile(QString filename_p)
{
    // load the data
    setlocale(LC_ALL, "C");

    osg::ref_ptr<osg::Node> node = m_osgwidget->createNodeFromFile(filename_p.toStdString());

    emit signal_add3DSceneToCartoView(node);
}
