#define M_PI 3.1415926535
#include "UserFormWidget.h"
#include "ui_UserFormWidget.h"

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

    // Default view is QImageView
    switchCartoViewTo(QImageView);

    _supportedRasterFormat << "tif" << "tiff";
    _supportedVectorFormat << "shp";
    _supported3DFileFormat << "obj" << "osg" << "ply" << "osgt";
    _supportedImageFormat << "jpg" << "jpeg" << "png";

    // Init QAction & Menu
    _extentAutoResize = new QAction(tr("Ajustement auto a l'emprise"),this);
    _extentAutoResize->setCheckable(true);
    _extentAutoResize->setChecked(true);

    _followLastItem = new QAction(tr("Suivre le dernier ajout"),this);
    _followLastItem->setCheckable(false);
    _followLastItem->setChecked(false);

    _manualMove = new QAction(tr("Deplacement manuel"),this);
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
#ifdef WITH_OSG
    qRegisterMetaType< osg::ref_ptr<osg::Node> >();
#endif

    connect(this,SIGNAL(signal_loadRasterFromFile(QString)),&_resultLoadingTask,SLOT(slot_loadRasterFromFile(QString)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_addRasterToCartoView(QgsRasterLayer*)),this,SLOT(slot_addRasterToCartoView(QgsRasterLayer*)),Qt::QueuedConnection);
    connect(this,SIGNAL(signal_load3DSceneFromFile(QString)),&_resultLoadingTask,SLOT(slot_load3DSceneFromFile(QString)),Qt::QueuedConnection);
#ifdef WITH_OSG
    connect(&_resultLoadingTask,SIGNAL(signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),this,SLOT(slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),Qt::QueuedConnection);
#endif
    _resultLoadingTask.moveToThread(&_resultLoadingThread);
    _resultLoadingThread.start();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slot_showMapContextMenu(const QPoint&)));

}

UserFormWidget::~UserFormWidget()
{
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

//    QAction *zoomOutAction = new QAction(this);
//    IconizedActionWrapper *zoomOutActionWrapper = new IconizedActionWrapper(zoomOutAction);
//    _iconFactory->attachIcon(zoomOutActionWrapper, "lnf/icons/carte-zoom-out.svg", false, false);
//    zoomOutAction->setIconText(tr("Zoom AR"));
//    _mapToolBar->addAction(zoomOutAction);
//    connect(zoomOutAction, SIGNAL(triggered(bool)), this, SLOT(slot_activateZoomOutTool()));
//    _zoomOutTool = new QgsMapToolZoom(_ui->_GRV_map, true);
//    _zoomOutTool->setAction(zoomOutAction);

    QAction *recenterAction = new QAction(this);
    IconizedActionWrapper *recenterActionWrapper = new IconizedActionWrapper(recenterAction);
    _iconFactory->attachIcon(recenterActionWrapper, "lnf/icons/carte-afficher-tout.svg", false, false);
    recenterAction->setIconText(tr("Recentrer"));
    _mapToolBar->addAction(recenterAction);
    connect(recenterAction, SIGNAL(triggered(bool)), this, SLOT(slot_recenterMap()));

//    _mapToolBar->setFloatable(true);
//    _mapToolBar->setMovable(true);
//    _mapToolBar->setOrientation(Qt::Vertical);
//    _mapToolBar->show();

    _ui->_WID_pageQGis->layout()->setMenuBar(_mapToolBar);
    _mapToolBar->setVisible(false);
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

void UserFormWidget::slot_activatePanTool()
{
//    _ui->_GRV_map->setMapTool(_panTool);
}

void UserFormWidget::slot_activateZoomInTool()
{
//    _ui->_GRV_map->setMapTool(_zoomInTool);
}

void UserFormWidget::slot_activateZoomOutTool()
{
//    _ui->_GRV_map->setMapTool(_zoomOutTool);
}

void UserFormWidget::slot_recenterMap()
{
    /* resize options ? */
    slot_onAutoResizeTrigger();
}

//void UserFormWidget::slot_layerWasAdded(QgsMapLayer *layer)
//{
//    QListWidgetItem *layerItem = new QListWidgetItem(layer->id());
//    layerItem->setCheckState(Qt::Checked);
//    layerItem->setToolTip(layer->name());
//    _layersWidget->addItem(layerItem);
//}

void UserFormWidget::slot_layerWasRemoved(QString layerId)
{
    QList<QListWidgetItem *> foundItems = _layersWidget->findItems(layerId, Qt::MatchExactly);

    if (foundItems.isEmpty()) {
        qWarning() << QString("Layer '%1' was not found in layers widget").arg(layerId);
        return;
    }

    if (foundItems.size() > 1) {
        qWarning() << QString("Found %1 layers with id '%2', they will all be removed").arg(foundItems.size()).arg(layerId);
    } else {
        qDebug() << QString("Removing layer '%1' from layers widget").arg(layerId);
    }

    foreach (QListWidgetItem *item, foundItems) {
        _layersWidget->removeItemWidget(item);
        delete item;
    }
}

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
#ifdef WITH_OSG
    _ui->_OSG_viewer->clearSceneData();
#endif
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

void UserFormWidget::slot_addRasterToCartoView(QgsRasterLayer * rasterLayer_p) {

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

//    // Add the raster Layer to the Layer Registry
//    // QGis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);
//    QgsProject::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);

//    // Add the layer to the Layer Set
//    // QGis 2.99
//    //_layers.append(QgsMapCanvasLayer(rasterLayer_p, TRUE));//bool visibility
//    _players.append(rasterLayer_p);//bool visibility

//    this->updateMapCanvasAndExtent(NULL);

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

//    QFileInfo fileInfo(filename);
//    QString layerName = fileInfo.fileName();
//    layerName.chop(4);
//    QgsVectorLayer * mypLayer = new QgsVectorLayer(filename, layerName, "ogr");

//    if (!mypLayer->isValid()) {
//        qCritical() << QString("Layer '%1' is NOT valid").arg(layerName);
//        return;
//    }

//    qDebug() << QString("Loading layer '%1'...").arg(layerName);

////    mypLayer->setCoordinateSystem();

//    // Add the Vector Layer to the Layer Registry
//    // Qgis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE, TRUE);
//    QgsProject::instance()->addMapLayer(mypLayer, TRUE, TRUE);

//    // Add the layer to the Layer Set
//    // Qgis 2.99
//    //_layers.append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility
//    _players.append(mypLayer);

//    this->updateMapCanvasAndExtent(NULL);
}

void UserFormWidget::load3DFile(QString filename_p)
{
    if (_currentViewType!=OpenSceneGraphView)
        switchCartoViewTo(OpenSceneGraphView);
#ifdef WITH_OSG
    emit signal_load3DSceneFromFile(filename_p);
#endif
}

#ifdef WITH_OSG
void UserFormWidget::slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p)
{
    _ui->_OSG_viewer->setSceneData(sceneData_p);
}
#endif

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

//    this->updateMapCanvasAndExtent(NULL);
//    _ui->_GRV_map->refresh();
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
//    QgsProject * project = QgsProject::instance();
//    project->setFileName(filename);
//    project->write();
}

void UserFormWidget::addPolygonToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);


//    QgsVectorLayer *polygonLayer;
//    bool layerAlreadyExists = false;
//    int i;

//    if (findLayerIndexFromName(layerName_p, i)){
//        // QGis 2.99
//        //polygonLayer = (QgsVectorLayer *)_layers.at(i).layer();
//        polygonLayer = (QgsVectorLayer *)_players.at(i);

//        layerAlreadyExists = true;
//    }else{
//        polygonLayer = new QgsVectorLayer("Polygon", layerName_p, "memory");
//    }

//    QgsVectorDataProvider* polygonLayerp = polygonLayer->dataProvider();
//    QgsFeatureList feats;

//    // QGis 2.99
//    //QgsPolyline firstPolyline;
//    QgsPolylineXY firstPolyline;

//    // QGis 2.99
//    //QgsPolygon qgsPolygon;
//    QgsPolygonXY qgsPolygon;

//    for(unsigned int j=0; j<polygon_p.contours()[0].x.size(); j++){
//        // QGis 2.99
//        //firstPolyline.append( QgsPoint(polygon_p.contours()[0].x[j],polygon_p.contours()[0].y[j]) );
//        QgsPoint p(polygon_p.contours()[0].x[j],polygon_p.contours()[0].y[j]);
//        firstPolyline.append( p );
//    }

//    qgsPolygon.append(firstPolyline);
//    QgsFeature feat;
//    // QGis 2.99
//    //feat.setGeometry(QgsGeometry::fromPolygon( qgsPolygon ));
//    feat.setGeometry(QgsGeometry::fromPolygonXY( qgsPolygon ));
//    feats.append(feat);

//    polygonLayerp->addFeatures(feats);
//    polygonLayer->updateExtents();


//    // Complete properties such as color
//    QgsStringMap props;
//    props.insert("color", polyInsideColor_p);
//    // Qgis 2.99
//    //QgsFillSymbolV2 *symbol = QgsFillSymbolV2::createSimple(props);
//    QgsFillSymbol *symbol = QgsFillSymbol::createSimple(props);
//    // Qgis 2.99
//    //QgsFeatureRendererV2 *polygonLayerr = new QgsSingleSymbolRendererV2(symbol);
//    QgsFeatureRenderer *polygonLayerr = new QgsSingleSymbolRenderer(symbol);

//    // Qgis 2.99
//    //polygonLayer->setRendererV2(polygonLayerr);
//    polygonLayer->setRenderer(polygonLayerr);

//    // Qgis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(polygonLayer, TRUE);
//    QgsProject::instance()->addMapLayer(polygonLayer, TRUE);

//    if(!layerAlreadyExists)
//    {
//        // Qgis 2.99
//        //_layers.append(QgsMapCanvasLayer(polygonLayer, TRUE));
//        _players.append(polygonLayer);
//    }

//    this->updateMapCanvasAndExtent(polygonLayer);

}

void UserFormWidget::addPolylineToMap(basicproc::Polygon &polygon_p, QString polyColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

//    QgsVectorLayer *polylineLayer;
//    bool layerAlreadyExists = false;
//    int i;

//    if (findLayerIndexFromName(layerName_p, i)){
//        // Qgis 2.99
//        //polylineLayer = (QgsVectorLayer *)_layers.at(i).layer();
//        polylineLayer = (QgsVectorLayer *)_players.at(i);

//        layerAlreadyExists = true;
//    }else{
//        polylineLayer = new QgsVectorLayer("LineString", layerName_p, "memory");
//    }

//    QgsVectorDataProvider* polylineLayerp = polylineLayer->dataProvider();
//    QgsFeatureList feats;

//    QgsPolyline qgsPolyline;

//    for(unsigned int j=0; j<polygon_p.contours()[0].x.size(); j++){
//        qgsPolyline.append( QgsPoint(polygon_p.contours()[0].x[j],polygon_p.contours()[0].y[j]) );
//    }
//    qgsPolyline.append( QgsPoint(polygon_p.contours()[0].x[0],polygon_p.contours()[0].y[0]) );

//    QgsFeature feat;
//    feat.setGeometry(QgsGeometry::fromPolyline( qgsPolyline ));
//    feats.append(feat);

//    polylineLayerp->addFeatures(feats);
//    polylineLayer->updateExtents();


//    // Complete properties such as color
//    QgsStringMap props;
//    props.insert("color", polyColor_p);
//    // Qgis 2.99
//    //QgsLineSymbolV2 *symbol = QgsLineSymbolV2::createSimple(props);
//    QgsLineSymbol *symbol = QgsLineSymbol::createSimple(props);
//    // Qgis 2.99
//    //QgsFeatureRendererV2 *polygonLayerr = new QgsSingleSymbolRendererV2(symbol);
//    QgsFeatureRenderer *polygonLayerr = new QgsSingleSymbolRenderer(symbol);

//    // Qgis 2.99
//    //polylineLayer->setRendererV2(polygonLayerr);
//    polylineLayer->setRenderer(polygonLayerr);

//    // Qgis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(polylineLayer, TRUE);
//    QgsProject::instance()->addMapLayer(polylineLayer, TRUE);

//    if(!layerAlreadyExists)
//    {
//        // Qgis 2.99
//        //_layers.append(QgsMapCanvasLayer(polylineLayer, TRUE));
//        _players.append(polylineLayer);
//    }

//    this->updateMapCanvasAndExtent(polylineLayer);
}

void UserFormWidget::exportMapViewToImage(QString imageFilePath)
{
    QImage image(QSize(800, 600), QImage::Format_ARGB32_Premultiplied);

    QColor color(0,0,0);
    image.fill(color.rgb());

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

//    // Qgis 2.99
//    //QgsMapRenderer renderer;
//    QgsMapSettings settings;
//    QList<QgsMapLayer *> layers = _ui->_GRV_map->layers();

//    QStringList layerIds;

//    // Qgis 2.99
////    foreach (QgsMapLayer *layer, layers) {
////        layerIds << layer->id();
////    }
////    renderer.setLayerSet(layerIds);
//    settings.setLayers(layers);

//    /* user current canvas extent to render image */
//    QgsRectangle rect(_ui->_GRV_map->extent());
//    //Qgis 2.99
//    //renderer.setExtent(rect);
//    settings.setExtent(rect);

//    //Qgis 2.99
//    //renderer.setOutputSize(image.size(), image.logicalDpiX());
//    settings.setOutputSize(image.size());
//    settings.setOutputDpi(image.logicalDpiX());

//    //Qgis 2.99
//    //renderer.render(&painter);
//    QgsMapRendererSequentialJob renderer(settings);
//    renderer.start();

    image.save(imageFilePath, "png");
}

//void UserFormWidget::addQGisPointsToMap(QList<QgsPoint> &pointsList_p, QString pointsColor_p, QString layerName_p){

//    if (_currentViewType!=QGisMapLayer)
//        switchCartoViewTo(QGisMapLayer);

//    QgsVectorLayer *pointsLayer;
//    bool layerAlreadyExists = false;
//    int i;

//    if (findLayerIndexFromName(layerName_p, i)){
//        // Qgis 2.99
//        //pointsLayer = (QgsVectorLayer *)_layers.at(i).layer();
//        pointsLayer = (QgsVectorLayer *)_players.at(i);

//        layerAlreadyExists = true;
//    }else{
//        pointsLayer = new QgsVectorLayer("Point", layerName_p, "memory");
//    }

//    QgsVectorDataProvider* pointsLayerp = pointsLayer->dataProvider();
//    QgsFeatureList feats;

//    // Fill feats with points
//    foreach (QgsPoint point, pointsList_p) {
//        QgsFeature feat;
//        // Qgis 2.99
//        //feat.setGeometry(QgsGeometry::fromPoint( point ));
//        feat.setGeometry(QgsGeometry::fromPointXY( point ));
//        feats.append(feat);
//    }

//    pointsLayerp->addFeatures(feats);
//    pointsLayer->updateExtents();

//    // Complete properties such as color
//    QgsStringMap props;
//    props.insert("name", "square");
//    props.insert("color", pointsColor_p);
//    // Qgis 2.99
//    //QgsMarkerSymbolV2 *symbol = QgsMarkerSymbolV2::createSimple(props);
//    QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple(props);
//    // Qgis 2.99
//    //QgsFeatureRendererV2 *pointsLayerr = new QgsSingleSymbolRendererV2(symbol);
//    QgsFeatureRenderer *pointsLayerr = new QgsSingleSymbolRenderer(symbol);

//    // Qgis 2.99
//    //pointsLayer->setRendererV2(pointsLayerr);
//    pointsLayer->setRenderer(pointsLayerr);

//    // Qgis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(pointsLayer, TRUE);
//    QgsProject::instance()->addMapLayer(pointsLayer, TRUE);

//    if (!layerAlreadyExists)
//    {
//        // Qgis 2.99
//        //_layers.append(QgsMapCanvasLayer(pointsLayer, TRUE));
//        _players.append(pointsLayer);
//    }

//    this->updateMapCanvasAndExtent(pointsLayer);

//}

void UserFormWidget::loadTestVectorLayer()
{

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);


//    //QList<QgsMapCanvasLayer> layers;
//    QgsVectorLayer *v1 = new QgsVectorLayer("Point", "temporary_points", "memory");

//    QgsVectorDataProvider* v1p = v1->dataProvider();

//    /*QList<QgsField> fields;
//    fields.append(QgsField("name", QVariant::String));
//    fields.append(QgsField("age", QVariant::Int));
//    fields.append(QgsField("size", QVariant::Double));

//    v1p->addAttributes(fields);
//    v1->updateFields();*/

//    QgsFeature feat1;
//    //Qgis 2.99
//    //feat1.setGeometry(QgsGeometry::fromPoint(QgsPoint(10,10)));
//    feat1.setGeometry(QgsGeometry::fromPointXY(QgsPoint(10,10)));

//    /*QgsAttributes attrs;
//    attrs.append("Johny");
//    attrs.append(2);
//    attrs.append(0.3);
//    feat.setAttributes(attrs);*/

//    QgsFeature feat2;
//    //Qgis 2.99
//    //feat2.setGeometry(QgsGeometry::fromPoint(QgsPoint(20,20)));
//    feat2.setGeometry(QgsGeometry::fromPointXY(QgsPoint(20,20)));

//    QgsFeatureList feats;
//    feats.append(feat1);
//    feats.append(feat2);
//    v1p->addFeatures(feats);
//    v1->updateExtents();

//    //QgsFeatureRendererV2 *v1r = v1->rendererV2();


//    QgsStringMap props;
//    props.insert("name", "square");
//    props.insert("color", "red");
//    //Qgis 2.99
//    //QgsMarkerSymbolV2 *symbol = QgsMarkerSymbolV2::createSimple(props);
//    QgsMarkerSymbol *symbol = QgsMarkerSymbol::createSimple(props);

//    //Qgis 2.99
//    //QgsFeatureRendererV2 *v1r = new QgsSingleSymbolRendererV2(symbol);
//    QgsFeatureRenderer *v1r = new QgsSingleSymbolRenderer(symbol);

//    //Qgis 2.99
//    //v1->setRendererV2(v1r);
//    v1->setRenderer(v1r);

//    //Qgis 2.99
//    //QgsMapLayerRegistry::instance()->addMapLayer(v1, TRUE);
//    QgsProject::instance()->addMapLayer(v1, TRUE);

//    //Qgis 2.99
//   // _layers.append(QgsMapCanvasLayer(v1, TRUE));
//    _players.append(v1);

//    this->updateMapCanvasAndExtent(NULL);

//    qDebug() << "RENDER VECTOR LAYER !";

}

void UserFormWidget::loadImageFile(QString filename){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);


    QImage result(filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

//void UserFormWidget::updateMapCanvasAndExtent(QgsMapLayer *currentLayer_p)
//{
//    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

//    // Merge extents
//    // Qgis 2.99
//    //QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
//    QMap<QString, QgsMapLayer*> layers = QgsProject::instance()->mapLayers();

//    QgsRectangle finalExtent, extent;
//    int i=0;

//    if (_repaintBehaviorState == ExtentAutoResize){

//        foreach (QgsMapLayer* layer, layers.values()) {
//            if (i==0) {
//                finalExtent = layer->extent();
//            }
//            else {
//                extent = layer->extent();
//                // Qgis 2.99
//                //finalExtent.combineExtentWith(&extent);
//                finalExtent.combineExtentWith(extent);
//            }
//            i++;
//        }

//        if (finalExtent.width()==0 && layers.size()==1){
//            finalExtent.setXMinimum(finalExtent.xMinimum()-1.0);
//            finalExtent.setYMinimum(finalExtent.yMinimum()-1.0);
//            finalExtent.setXMaximum(finalExtent.xMaximum()+1.0);
//            finalExtent.setYMaximum(finalExtent.yMaximum()+1.0);
//        }
//        mapCanvas->setExtent(finalExtent);
//    }else if (_repaintBehaviorState == FollowLastItem){
//        if (currentLayer_p !=NULL){
//            extent = currentLayer_p->extent();

//            if (extent.width()==0){
//                extent.setXMinimum(extent.xMinimum()-1.0);
//                extent.setYMinimum(extent.yMinimum()-1.0);
//                extent.setXMaximum(extent.xMaximum()+1.0);
//                extent.setYMaximum(extent.yMaximum()+1.0);
//            }

//            mapCanvas->setExtent(extent);
//        }
//    }else if (_repaintBehaviorState == ManualMove){
//        // do nothing
//    }

//    // Qgis 2.99
//    //mapCanvas->setLayerSet(_layers);
//    mapCanvas->setLayers(_players);

//    if (currentLayer_p != NULL)
//        mapCanvas->setCurrentLayer(currentLayer_p);

//    mapCanvas->refresh();

//#ifdef WIN32
//    mapCanvas->update();
//#endif

//    this->repaint();
//    QApplication::processEvents();
//    QApplication::flush();

//}

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

    cvtColor(*(image->imageData()), dest,CV_BGR2RGB);

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

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
    QFileInfo fileInfo(filename_p);

//    QgsRasterLayer * rasterLayer = new QgsRasterLayer(filename_p, fileInfo.fileName());

//    if(rasterLayer)
//    {
//        if (rasterLayer->isValid())
//        {
//            qDebug("QGis raster layer is valid");
//            emit signal_addRasterToCartoView(rasterLayer);
//        }
//        else
//        {
//            qDebug("QGis raster layer is NOT valid");
//            delete rasterLayer;
//            return;
//        }
//    }

}

void resultLoadingTask::slot_load3DSceneFromFile(QString filename_p)
{
#ifdef WITH_OSG
    // load the data
    setlocale(LC_ALL, "C");
    //_loadedModel = osgDB::readRefNodeFile(sceneFile_p, new osgDB::Options("noTriStripPolygons"));
    osg::ref_ptr<osg::Node> sceneData = osgDB::readRefNodeFile(filename_p.toStdString());

    if (!sceneData)
    {
        std::cout << "No 3D data loaded" << std::endl;
        return;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(sceneData.get());

    emit signal_add3DSceneToCartoView(sceneData);
#endif
}

