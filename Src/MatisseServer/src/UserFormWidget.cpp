#include <qgsvectorlayer.h>
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
    _layers = new QList<QgsMapCanvasLayer>();

    // Default view is QImageView
    switchCartoViewTo(QImageView);

    _supportedRasterFormat << "tif" << "tiff";
    _supportedVectorFormat << "shp";
    _supported3DFileFormat << "obj" << "osg" << "ply" << "osgt";
    _supportedImageFormat << "jpg" << "jpeg" << "png";

    // Init loading thread
    qRegisterMetaType< osg::ref_ptr<osg::Node> >();

    connect(this,SIGNAL(signal_loadRasterFromFile(QString)),&_resultLoadingTask,SLOT(slot_loadRasterFromFile(QString)));
    connect(&_resultLoadingTask,SIGNAL(signal_addRasterToCartoView(QgsRasterLayer*)),this,SLOT(slot_addRasterToCartoView(QgsRasterLayer*)));
    connect(this,SIGNAL(signal_load3DSceneFromFile(QString)),&_resultLoadingTask,SLOT(slot_load3DSceneFromFile(QString)));
    connect(&_resultLoadingTask,SIGNAL(signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)),this,SLOT(slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node>)));

    _resultLoadingTask.moveToThread(&_resultLoadingThread);
    _resultLoadingThread.start();

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
    _layers->clear();
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
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
    //showUserParameters(false);
    clear();
}


void UserFormWidget::loadRasterFile(QString filename) {

    emit signal_loadRasterFromFile(filename);

}

void UserFormWidget::slot_addRasterToCartoView(QgsRasterLayer * rasterLayer_p) {


    // Add the raster Layer to the Layer Registry
    QgsMapLayerRegistry::instance()->addMapLayer(rasterLayer_p, TRUE, TRUE);

    // Add the layer to the Layer Set
    _layers->append(QgsMapCanvasLayer(rasterLayer_p, TRUE));//bool visibility

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle extent;
    foreach (QgsMapLayer* layer, layers.values()) {
        if (extent.width()==0) {
            extent = layer->extent();
        }
        else {
            extent.combineExtentWith(&layer->extent());
        }
    }

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    // set the canvas to the extent of our layer
    mapCanvas->setExtent(extent);

    // Set the Map Canvas Layer Set
    mapCanvas->setLayerSet(*_layers);
    mapCanvas->refresh();
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
    _layers->append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle extent;
    foreach (QgsMapLayer* layer, layers.values()) {
        if (extent.width()==0) {
            extent = layer->extent();
        }
        else {
            extent.combineExtentWith(&layer->extent());
        }
    }


    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    // set the canvas to the extent of our layer
    mapCanvas->setExtent(extent);

    // Set the Map Canvas Layer Set
    mapCanvas->setLayerSet(*_layers);
    mapCanvas->refresh();
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

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    QgsVectorLayer *polygonLayer = new QgsVectorLayer("Polygon", layerName_p, "memory");
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

    QgsMapCanvasLayer polygonLayerWrap(polygonLayer);
    _layers->append(polygonLayerWrap);
    mapCanvas->setLayerSet(*_layers);

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle extent;
    foreach (QgsMapLayer* layer, layers.values()) {
        if (extent.width()==0) {
            extent = layer->extent();
        }
        else {
            extent.combineExtentWith(&layer->extent());
        }
    }
    mapCanvas->setExtent(extent);

    mapCanvas->setCurrentLayer(polygonLayer);
    mapCanvas->refresh();

}

void UserFormWidget::addQGisPointsToMap(QList<QgsPoint> &pointsList_p, QString pointsColor_p, QString layerName_p){

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    QgsVectorLayer *pointsLayer = new QgsVectorLayer("Point", layerName_p, "memory");
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

    QgsMapCanvasLayer pointsLayerWrap(pointsLayer);
    _layers->append(pointsLayerWrap);
    mapCanvas->setLayerSet(*_layers);

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle extent;
    foreach (QgsMapLayer* layer, layers.values()) {
        if (extent.width()==0) {
            extent = layer->extent();
        }
        else {
            extent.combineExtentWith(&layer->extent());
        }
    }
    mapCanvas->setExtent(extent);

    mapCanvas->setCurrentLayer(pointsLayer);
    mapCanvas->refresh();

    qDebug() << "RENDER POINTS LAYER !";

}

void UserFormWidget::loadTestVectorLayer()
{

    if (_currentViewType!=QGisMapLayer)
        switchCartoViewTo(QGisMapLayer);

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

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

    QgsMapCanvasLayer v1Wrap(v1);
    _layers->append(v1Wrap);
    mapCanvas->setLayerSet(*_layers);

    // Merge extents
    QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
    QgsRectangle extent;
    foreach (QgsMapLayer* layer, layers.values()) {
        if (extent.width()==0) {
            extent = layer->extent();
        }
        else {
            extent.combineExtentWith(&layer->extent());
        }
    }
    mapCanvas->setExtent(extent);

    mapCanvas->setCurrentLayer(v1);
    mapCanvas->refresh();

    qDebug() << "RENDER VECTOR LAYER !";

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


void UserFormWidget::loadImageFile(QString filename){

    if (_currentViewType!=QImageView)
        switchCartoViewTo(QImageView);


    QImage result(filename);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

// Threaded result file loading task

resultLoadingTask::resultLoadingTask()
{
    _lastLoadedView = QImageView;

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
