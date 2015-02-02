#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmaplayerregistry.h>


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
        _ui->_SCA_parameters->setWidget(_parametersWidget);

    } else {
        _ui->_SCA_parameters->setWidget(NULL);
    }
}

void UserFormWidget::showQGisCanvas(bool flag)
{
    if (flag) {
        _ui->_stackedWidget->setCurrentIndex(0);
    }
    else {
        _ui->_stackedWidget->setCurrentIndex(1);
    }

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
    showUserParameters(false);
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

void UserFormWidget::displayImage(Image *image ){

    Mat dest;

    qDebug()<< "Channels " << image->imageData()->channels();

    cvtColor(*(image->imageData()), dest,CV_BGR2RGB);

    QImage result((uchar*) dest.data, dest.cols, dest.rows, dest.step, QImage::Format_RGB888);
    const QPixmap pix = QPixmap::fromImage(result);
    const QSize size = _ui->_label->size();
    this->_ui->_label->setPixmap(pix.scaled(size,Qt::KeepAspectRatio));

}

