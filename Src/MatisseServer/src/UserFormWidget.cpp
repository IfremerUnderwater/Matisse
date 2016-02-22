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
    switch(cartoViewType_p)
    {

    case QGisMapLayer:
        _ui->_stackedWidget->setCurrentIndex(0);
        break;
    case QImageView:
        _ui->_stackedWidget->setCurrentIndex(1);
        break;
    case OpenSceneGraphView:
        _ui->_stackedWidget->setCurrentIndex(2);
        _ui->_OSG_viewer->setSceneFromFile("./3DTestData/wallMeshTex.obj");
        break;

    }

}


void UserFormWidget::createCanvas() {

    qDebug() << "Create QGIS Canvas";

    // _ui->_GRV_map->deleteLater();

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;
    //QgsMapCanvas* mapCanvas= new QgsMapCanvas(NULL, "mapCanvas");
    mapCanvas->enableAntiAliasing(true);
    mapCanvas->useImageToRender(false);
    mapCanvas->setCanvasColor(QColor(MATISSE_BLACK));
    mapCanvas->freeze(false);
    mapCanvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapCanvas->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapCanvas->refresh();
    mapCanvas->show();

    //_ui->_SPL_user->insertWidget(0, mapCanvas);
    // QList<int> heights;
    // heights.push_back(550);
    // heights.push_back(550);
    // heights.push_back(430);
    // _ui->_SPL_user->setSizes(heights);

    //    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //    sizePolicy.setHorizontalStretch(0);
    //    sizePolicy.setVerticalStretch(0);
    //    sizePolicy.setHeightForWidth(mapCanvas->sizePolicy().hasHeightForWidth());
    //    _ui->_GRV_map->setSizePolicy(sizePolicy);

    //_ui->_GRV_map=mapCanvas;

    //_ui->_SPL_user->setStretchFactor(0, 1);
    //_ui->_SPL_user->setStretchFactor(1, 1);

}

void UserFormWidget::clear()
{
    _layers->clear();
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    _ui->_GRV_map->clearExtentHistory();
    _ui->_GRV_map->clear();
    _ui->_GRV_map->refresh();

}

void UserFormWidget::resetJobForm()
{
    // reset parameters
    qDebug() << "resetJobForm";
    //showUserParameters(false);
    clear();
}


void UserFormWidget::loadRasterFile(QString filename) {

    if (filename.isEmpty()) {
        return;
    }
    QFileInfo fileInfo(filename);

    QgsRasterLayer * mypLayer = new QgsRasterLayer(filename, fileInfo.fileName());

    if (mypLayer->isValid())
    {
        qDebug("Layer is valid");
    }
    else
    {
        qDebug("Layer is NOT valid");
        return;
    }

    // Add the raster Layer to the Layer Registry
    QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE, TRUE);

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

void UserFormWidget::loadShapefile(QString filename)
{
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

void UserFormWidget::loadTestVectorLayer()
{
    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    //QList<QgsMapCanvasLayer> layers;
    QgsVectorLayer *v1 = new QgsVectorLayer("Point", "temporary_points", "memory");

    QgsVectorDataProvider* v1p = v1->dataProvider();

    QList<QgsField> fields;
    fields.append(QgsField("name", QVariant::String));
    fields.append(QgsField("age", QVariant::Int));
    fields.append(QgsField("size", QVariant::Double));

    v1p->addAttributes(fields);
    v1->updateFields();

    QgsFeature feat;
    feat.setGeometry(QgsGeometry::fromPoint(QgsPoint(10,10)));
    QgsAttributes attrs;
    attrs.append("Johny");
    attrs.append(2);
    attrs.append(0.3);
    feat.setAttributes(attrs);
    QgsFeatureList feats;
    feats.append(feat);
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
    //    mapCanvas->setCurrentLayer(v1);
    mapCanvas->refresh();

    qDebug() << "RENDER VECTOR LAYER !";

}

void UserFormWidget::slot_parametersChanged(bool changed)
{
    // pour transmettre le signal vers la maun gui...
    qDebug() << "Emit param changed userForm...";
    emit signal_parametersChanged(changed);
}

void UserFormWidget::displayImage(Image *image ){

    Mat dest;

    qDebug()<< "Channels " << image->imageData()->channels();

    cvtColor(*(image->imageData()), dest,CV_BGR2RGB);

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_LA_resultImage->size();
    this->_ui->_LA_resultImage->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

