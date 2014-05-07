#include "UserFormWidget.h"
#include "ui_UserFormWidget.h"
#include "qgsmapcanvas.h"
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayerregistry.h>


UserFormWidget::UserFormWidget(QWidget *parent) :
    QWidget(parent),
    _ui(new Ui::UserFormWidget),
    _parametersWidget(NULL)
{
    _ui->setupUi(this);
    init();
}

UserFormWidget::~UserFormWidget()
{
    delete _ui;
}

void UserFormWidget::showUserParameters(Tools * tools)
{
    if (_parametersWidget) {
        _parametersWidget->deleteLater();
        _parametersWidget = NULL;
    }
    if (tools) {
        _parametersWidget = tools->createFullParametersDialog(true);
        connect(_parametersWidget, SIGNAL(signal_valuesModified(bool)), this, SLOT(slot_parametersChanged(bool)));
        _ui->_SCA_parameters->setWidget(_parametersWidget);

    } else {
        _ui->_SCA_parameters->setWidget(NULL);
    }
}

void UserFormWidget::init() {
}


void UserFormWidget::createCanvas() {

    qDebug() << "Create QGIS Canvas";

   // _ui->_GRV_map->deleteLater();

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;
    //QgsMapCanvas* mapCanvas= new QgsMapCanvas(NULL, "mapCanvas");
    mapCanvas->enableAntiAliasing(true);
    mapCanvas->useImageToRender(false);
    mapCanvas->setCanvasColor(Qt::white);
    mapCanvas->freeze(false);
    mapCanvas->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapCanvas->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

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

    _ui->_SPL_user->setStretchFactor(0, 1);
    _ui->_SPL_user->setStretchFactor(1, 1);

}

void UserFormWidget::clear()
{
    QgsMapLayerRegistry::instance()->removeAllMapLayers();
    _ui->_GRV_map->clearExtentHistory();
    _ui->_GRV_map->clear();
    _ui->_GRV_map->refresh();
}

void UserFormWidget::resetJobForm()
{
    // reset parameters
    showUserParameters();
    clear();
}

void UserFormWidget::loadVectorFile(QString filename) {

    if (filename.isEmpty()) {
        clear();
        return;
    }

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
    QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE, TRUE);


    // Add the layers to the Layer Set
    QList<QgsMapCanvasLayer> myList;
    myList.append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    // set the canvas to the extent of our layer
    mapCanvas->setExtent(mypLayer->extent());
    // Set the Map Canvas Layer Set
    mapCanvas->setLayerSet(myList);
    mapCanvas->refresh();
}

void UserFormWidget::loadRasterFile(QString filename) {
    if (filename.isEmpty()) {
        clear();
        return;
    }
    QFileInfo fileInfo(filename);
    QString csrString = "PROJCS[\"UTM Zone 31, Northern Hemisphere\", GEOGCS[\"WGS 84\",DATUM[\"unknown\",SPHEROID[\"WGS84\",6378137,298.257223563]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]], PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0], PARAMETER[\"central_meridian\",3],  PARAMETER[\"scale_factor\",0.9996], PARAMETER[\"false_easting\",500000], PARAMETER[\"false_northing\",0],UNIT[\"metre\",1, AUTHORITY[\"EPSG\",\"9001\"]]]";

    QgsRasterLayer * mypLayer = new QgsRasterLayer(filename, fileInfo.fileName());

    QgsCoordinateReferenceSystem csr;
    //csr.createFromWkt(csrString);

//   csr.createFromProj4("+proj=utm +zone=31 +units=m +datum=WGS84 +no_defs");
//    if (csr.isValid())
//    {
//      qDebug("Proj is valid");
//    }
//    else
//    {
//      qDebug("Proj is NOT valid");
//      return;
//    }

//    mypLayer->setCrs(csr);

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
    QgsMapLayerRegistry::instance()->addMapLayer(mypLayer, TRUE, TRUE);


    // Add the layers to the Layer Set
    QList<QgsMapCanvasLayer> myList;
    myList.append(QgsMapCanvasLayer(mypLayer, TRUE));//bool visibility

    QgsMapCanvas* mapCanvas = _ui->_GRV_map;

    // set the canvas to the extent of our layer
    mapCanvas->setExtent(mypLayer->extent());
    // Set the Map Canvas Layer Set
    mapCanvas->setLayerSet(myList);
    mapCanvas->refresh();
}

void UserFormWidget::setTools(Tools *tools)
{
    _tools = tools;
}

ParametersWidgetSkeleton *UserFormWidget::parametersWidget()
{
    return _parametersWidget;
}

void UserFormWidget::slot_parametersChanged(bool changed)
{
    // pour transmettre le signal vers la maun gui...
    qDebug() << "Emit param changed userForm...";
    emit signal_parametersChanged(changed);
}

