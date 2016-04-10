#include <qgsvectorlayer.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayerregistry.h>

#include <qgsvectordataprovider.h>
#include <qgsgeometry.h>
#include <qgsmarkersymbollayerv2.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgsrendererv2.h>
#include <qgsproject.h>


#include "UserFormWidget.h"
#include "ui_UserFormWidget.h"
#include "qgsmapcanvas.h"

#include <opencv2/opencv.hpp>

using namespace cv;


UserFormWidget::UserFormWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::UserFormWidget),
    _parametersWidget(NULL),
    _tools(NULL)
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
    qRegisterMetaType< osg::ref_ptr<osg::Node> >();

    connect(this,SIGNAL(signal_loadRasterFromFile(QString)),&_resultLoadingTask,SLOT(slot_loadRasterFromFile(QString)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_addRasterToCartoView(QgsRasterLayer*)),this,SLOT(slot_addRasterToCartoView(QgsRasterLayer*)),Qt::QueuedConnection);
    connect(this,SIGNAL(signal_load3DSceneFromFile(QString)),&_resultLoadingTask,SLOT(slot_load3DSceneFromFile(QString)),Qt::QueuedConnection);
    connect(&_resultLoadingTask,SIGNAL(signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),this,SLOT(slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),Qt::QueuedConnection);

    _resultLoadingTask.moveToThread(&_resultLoadingThread);
    _resultLoadingThread.start();

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(slot_showContextMenu(const QPoint&)));

}

UserFormWidget::~UserFormWidget()
{
    delete _ui;
}

void UserFormWidget::showUserParameters(bool flag)
{
    if (!_tools) {
        return;
    }
    _tools->eraseDialog();
    if (_parametersWidget) {
        _parametersWidget->deleteLater();
        _parametersWidget = NULL;
    }
    if (flag) {
        _parametersWidget = _tools->createFullParametersDialog(true);
        connect(_parametersWidget, SIGNAL(signal_valuesModified(bool)), this, SLOT(slot_parametersChanged(bool)));
        //_ui->_SCA_parameters->setWidget(_parametersWidget);

    } else {
        //_ui->_SCA_parameters->setWidget(NULL);
    }
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


void UserFormWidget::createCanvas() {

    qDebug() << "Set QGIS Canvas properties";

    // _ui->_GRV_map->deleteLater();

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    mapCanvas->enableAntiAliasing(true);
    mapCanvas->useImageToRender(false);
    mapCanvas->setCanvasColor(QColor(MATISSE_BLACK));
    mapCanvas->freeze(false);
    mapCanvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapCanvas->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapCanvas->refresh();
    mapCanvas->show();

}

void UserFormWidget::clear()
{
    // Clear QGis Widget
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    _layers.clear();
    _ui->_GRV_map->clearExtentHistory();
    _ui->_GRV_map->clear();
    _ui->_GRV_map->refresh();

    // Clear OSG Widget
    _ui->_OSG_viewer->clearSceneData();

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


    // Add the raster Layer to the Layer Registry
    QgsMapLayerRegistry::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);

    // Add the layer to the Layer Set
    _layers.append(QgsMapCanvasLayer(rasterLayer_p, TRUE));//bool visibility

    this->updateMapCanvasAndExtent(NULL);

}

void UserFormWidget::loadShapefile(QString filename)
{

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    QFileInfo fileInfo(filename);
    QgsVectorLayer * mypLayer = new QgsVectorLayer(filename, fileInfo.fileName(), "ogr");
    if (mypLayer->isValid())
    {
        qDebug("Layer is valid");
    }
    else
    {
        qDebug("Layer is NOT valid");
        return;
    }

    // Add the Vector Layer to the Layer Registry
    QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE);

    // Add the layer to the Layer Set
    _layers.append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility

    this->updateMapCanvasAndExtent(NULL);
}

void UserFormWidget::load3DFile(QString filename_p)
{
    if (_currentViewType!=OpenSceneGraphView)
        switchCartoViewTo(OpenSceneGraphView);

    emit signal_load3DSceneFromFile(filename_p);
}

void UserFormWidget::slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p)
{
    _ui->_OSG_viewer->setSceneData(sceneData_p);
}

void UserFormWidget::slot_showContextMenu(const QPoint &pos_p)
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

    this->updateMapCanvasAndExtent(NULL);
    _ui->_GRV_map->refresh();
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

void UserFormWidget::setTools(Tools *tools)
{
    _tools = tools;
}

void UserFormWidget::saveQgisProject(QString filename)
{
    QgsProject * project = QgsProject::instance();
    project->setFileName(filename);
    project->write();
}

void UserFormWidget::addPolygonToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);


    QgsVectorLayer *polygonLayer;
    bool layerAlreadyExists = false;
    int i;

    if (findLayerIndexFromName(layerName_p, i)){
        polygonLayer = (QgsVectorLayer *)_layers.at(i).layer();
        layerAlreadyExists = true;
    }else{
        polygonLayer = new QgsVectorLayer("Polygon", layerName_p, "memory");
    }

    QgsVectorDataProvider* polygonLayerp = polygonLayer->dataProvider();
    QgsFeatureList feats;

    QgsPolyline firstPolyline;
    QgsPolygon qgsPolygon;

    for(unsigned int j=0; j<polygon_p.contours()[0].x.size(); j++){
        firstPolyline.append( QgsPoint(polygon_p.contours()[0].x[j],polygon_p.contours()[0].y[j]) );
    }

    qgsPolygon.append(firstPolyline);
    QgsFeature feat;
    feat.setGeometry(QgsGeometry::fromPolygon( qgsPolygon ));
    feats.append(feat);

    polygonLayerp->addFeatures(feats);
    polygonLayer->updateExtents();


    // Complete properties such as color
    QgsStringMap props;
    props.insert("color", polyInsideColor_p);
    QgsFillSymbolV2 *symbol = QgsFillSymbolV2::createSimple(props);
    QgsFeatureRendererV2 *polygonLayerr = new QgsSingleSymbolRendererV2(symbol);

    polygonLayer->setRendererV2(polygonLayerr);


    QgsMapLayerRegistry::instance()->addMapLayer(polygonLayer, TRUE);

    if(!layerAlreadyExists)
        _layers.append(QgsMapCanvasLayer(polygonLayer, TRUE));

    this->updateMapCanvasAndExtent(polygonLayer);

}

void UserFormWidget::addPolylineToMap(basicproc::Polygon &polygon_p, QString polyColor_p, QString layerName_p){

    if (polygon_p.contours().size()>1){
        qDebug() << "Multi contours polygon not supported yet...returning !";
        return;
    }

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    QgsVectorLayer *polylineLayer;
    bool layerAlreadyExists = false;
    int i;

    if (findLayerIndexFromName(layerName_p, i)){
        polylineLayer = (QgsVectorLayer *)_layers.at(i).layer();
        layerAlreadyExists = true;
    }else{
        polylineLayer = new QgsVectorLayer("LineString", layerName_p, "memory");
    }

    QgsVectorDataProvider* polylineLayerp = polylineLayer->dataProvider();
    QgsFeatureList feats;

    QgsPolyline qgsPolyline;

    for(unsigned int j=0; j<polygon_p.contours()[0].x.size(); j++){
        qgsPolyline.append( QgsPoint(polygon_p.contours()[0].x[j],polygon_p.contours()[0].y[j]) );
    }
    qgsPolyline.append( QgsPoint(polygon_p.contours()[0].x[0],polygon_p.contours()[0].y[0]) );

    QgsFeature feat;
    feat.setGeometry(QgsGeometry::fromPolyline( qgsPolyline ));
    feats.append(feat);

    polylineLayerp->addFeatures(feats);
    polylineLayer->updateExtents();


    // Complete properties such as color
    QgsStringMap props;
    props.insert("color", polyColor_p);
    QgsLineSymbolV2 *symbol = QgsLineSymbolV2::createSimple(props);
    QgsFeatureRendererV2 *polygonLayerr = new QgsSingleSymbolRendererV2(symbol);

    polylineLayer->setRendererV2(polygonLayerr);


    QgsMapLayerRegistry::instance()->addMapLayer(polylineLayer, TRUE);

    if(!layerAlreadyExists)
        _layers.append(QgsMapCanvasLayer(polylineLayer, TRUE));

    this->updateMapCanvasAndExtent(polylineLayer);

}

void UserFormWidget::addQGisPointsToMap(QList<QgsPoint> &pointsList_p, QString pointsColor_p, QString layerName_p){

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    QgsVectorLayer *pointsLayer;
    bool layerAlreadyExists = false;
    int i;

    if (findLayerIndexFromName(layerName_p, i)){
        pointsLayer = (QgsVectorLayer *)_layers.at(i).layer();
        layerAlreadyExists = true;
    }else{
        pointsLayer = new QgsVectorLayer("Point", layerName_p, "memory");
    }

    QgsVectorDataProvider* pointsLayerp = pointsLayer->dataProvider();
    QgsFeatureList feats;

    // Fill feats with points
    foreach (QgsPoint point, pointsList_p) {
        QgsFeature feat;
        feat.setGeometry(QgsGeometry::fromPoint( point ));
        feats.append(feat);
    }

    pointsLayerp->addFeatures(feats);
    pointsLayer->updateExtents();

    // Complete properties such as color
    QgsStringMap props;
    props.insert("name", "square");
    props.insert("color", pointsColor_p);
    QgsMarkerSymbolV2 *symbol = QgsMarkerSymbolV2::createSimple(props);
    QgsFeatureRendererV2 *pointsLayerr = new QgsSingleSymbolRendererV2(symbol);

    pointsLayer->setRendererV2(pointsLayerr);

    QgsMapLayerRegistry::instance()->addMapLayer(pointsLayer, TRUE);

    if (!layerAlreadyExists)
        _layers.append(QgsMapCanvasLayer(pointsLayer, TRUE));

    this->updateMapCanvasAndExtent(pointsLayer);

}

void UserFormWidget::loadTestVectorLayer()
{

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);


    //QList<QgsMapCanvasLayer> layers;
    QgsVectorLayer *v1 = new QgsVectorLayer("Point", "temporary_points", "memory");

    QgsVectorDataProvider* v1p = v1->dataProvider();

    /*QList<QgsField> fields;
    fields.append(QgsField("name", QVariant::String));
    fields.append(QgsField("age", QVariant::Int));
    fields.append(QgsField("size", QVariant::Double));

    v1p->addAttributes(fields);
    v1->updateFields();*/

    QgsFeature feat1;
    feat1.setGeometry(QgsGeometry::fromPoint(QgsPoint(10,10)));
    /*QgsAttributes attrs;
    attrs.append("Johny");
    attrs.append(2);
    attrs.append(0.3);
    feat.setAttributes(attrs);*/

    QgsFeature feat2;
    feat2.setGeometry(QgsGeometry::fromPoint(QgsPoint(20,20)));

    QgsFeatureList feats;
    feats.append(feat1);
    feats.append(feat2);
    v1p->addFeatures(feats);
    v1->updateExtents();

    //QgsFeatureRendererV2 *v1r = v1->rendererV2();


    QgsStringMap props;
    props.insert("name", "square");
    props.insert("color", "red");
    QgsMarkerSymbolV2 *symbol = QgsMarkerSymbolV2::createSimple(props);

    QgsFeatureRendererV2 *v1r = new QgsSingleSymbolRendererV2(symbol);
    //v1r->setSymbol(symbol);

    v1->setRendererV2(v1r);

    QgsMapLayerRegistry::instance()->addMapLayer(v1, TRUE);

    _layers.append(QgsMapCanvasLayer(v1, TRUE));

    this->updateMapCanvasAndExtent(NULL);

    qDebug() << "RENDER VECTOR LAYER !";

}

void UserFormWidget::loadImageFile(QString filename){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);


    QImage result(filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

void UserFormWidget::updateMapCanvasAndExtent(QgsMapLayer *currentLayer_p)
{
    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle finalExtent, extent;
    int i=0;

    if (_repaintBehaviorState == ExtentAutoResize){

        foreach (QgsMapLayer* layer, layers.values()) {
            if (i==0) {
                finalExtent = layer->extent();
            }
            else {
                extent = layer->extent();
                finalExtent.combineExtentWith(&extent);
            }
            i++;
        }

        if (finalExtent.width()==0 && layers.size()==1){
            finalExtent.setXMinimum(finalExtent.xMinimum()-1.0);
            finalExtent.setYMinimum(finalExtent.yMinimum()-1.0);
            finalExtent.setXMaximum(finalExtent.xMaximum()+1.0);
            finalExtent.setYMaximum(finalExtent.yMaximum()+1.0);
        }
        mapCanvas->setExtent(finalExtent);
    }else if (_repaintBehaviorState == FollowLastItem){
        if (currentLayer_p !=NULL){
            extent = currentLayer_p->extent();

            if (extent.width()==0){
                extent.setXMinimum(extent.xMinimum()-1.0);
                extent.setYMinimum(extent.yMinimum()-1.0);
                extent.setXMaximum(extent.xMaximum()+1.0);
                extent.setYMaximum(extent.yMaximum()+1.0);
            }

            mapCanvas->setExtent(extent);
        }
    }else if (_repaintBehaviorState == ManualMove){
        // do nothing
    }

    mapCanvas->setLayerSet(_layers);

    if (currentLayer_p != NULL)
        mapCanvas->setCurrentLayer(currentLayer_p);

    mapCanvas->refresh();

#ifdef WIN32
    mapCanvas->update();
#endif

    this->repaint();
    QApplication::processEvents();
    QApplication::flush();

}

bool UserFormWidget::findLayerIndexFromName(const QString &layerName_p, int &idx_p)
{
    int i=0;

    foreach (QgsMapCanvasLayer currentLayer, _layers){
        if(currentLayer.layer()->name() == layerName_p){
            idx_p = i;
            return true;
        }

        i++;
    }

    return false;

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


void UserFormWidget::slot_parametersChanged(bool changed)
{
    // pour transmettre le signal vers la maun gui...
    qDebug() << "Emit param changed userForm...";
    emit signal_parametersChanged(changed);
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

    QgsRasterLayer * rasterLayer = new QgsRasterLayer(filename_p, fileInfo.fileName());

    if(rasterLayer)
    {
        if (rasterLayer->isValid())
        {
            qDebug("QGis raster layer is valid");
            emit signal_addRasterToCartoView(rasterLayer);
        }
        else
        {
            qDebug("QGis raster layer is NOT valid");
            delete rasterLayer;
            return;
        }
    }

}

void resultLoadingTask::slot_load3DSceneFromFile(QString filename_p)
{

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

}

