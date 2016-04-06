#ifndef USERFORMWIDGET_H
#define USERFORMWIDGET_H

#include <QWidget>
#include <QImage>
#include <QGraphicsView>
#include <QThread>
#include <qgsmapcanvas.h>
#include "Image.h"
#include "Tools.h"
#include "GraphicalCharter.h"

#include "Polygon.h"

#include <osg/ref_ptr>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

Q_DECLARE_METATYPE(osg::ref_ptr<osg::Node>)

using namespace MatisseTools;
using namespace MatisseCommon;
namespace Ui {
class UserFormWidget;
}
class QgsRasterLayer;


enum CartoViewType { QGisMapLayer, QImageView, OpenSceneGraphView };


class resultLoadingTask : public QObject{
    Q_OBJECT

public:
    explicit resultLoadingTask();
    virtual ~resultLoadingTask();

signals:
    void signal_addRasterToCartoView(QgsRasterLayer * rasterLayer_p);
    void signal_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p);

public slots:
    void slot_loadRasterFromFile(QString filename_p = "");
    void slot_load3DSceneFromFile(QString filename_p = "");

private:



};


class UserFormWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit UserFormWidget(QWidget *parent = NULL);
    virtual ~UserFormWidget();

    void showUserParameters(bool flag);
    void switchCartoViewTo(CartoViewType cartoViewType_p);

    void createCanvas();
    void clear();
    void displayImage(Image *image);
    void resetJobForm();
    void loadRasterFile(QString filename = "");
    void loadShapefile(QString filename = "");
    void load3DFile(QString filename_p = "");
    void loadImageFile(QString filename);
    void setTools(Tools * tools);
    void saveQgisProject(QString filename);
    void loadTestVectorLayer();
    void addQGisPointsToMap(QList<QgsPoint> &pointsList_p, QString pointsColor_p, QString layerName_p);
    void addPolygonToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p);
    void addPolylineToMap(basicproc::Polygon &polygon_p, QString polyInsideColor_p, QString layerName_p);

    CartoViewType currentViewType() const;

    QStringList supportedRasterFormat() const;
    QStringList supportedVectorFormat() const;
    QStringList supported3DFileFormat() const;
    QStringList supportedImageFormat() const;

private:

    void updateMapCanvasAndExtent(QgsMapLayer *currentLayer_p);

    Ui::UserFormWidget *_ui;
    ParametersWidgetSkeleton * _parametersWidget;
    Tools * _tools;
    QList<QgsMapCanvasLayer> *_layers;

    CartoViewType _currentViewType;

    QThread _resultLoadingThread;
    resultLoadingTask _resultLoadingTask;

    QStringList _supportedRasterFormat;
    QStringList _supportedVectorFormat;
    QStringList _supported3DFileFormat;
    QStringList _supportedImageFormat;

protected slots:
    void slot_parametersChanged(bool changed);
    void slot_addRasterToCartoView(QgsRasterLayer * rasterLayer_p);
    void slot_add3DSceneToCartoView(osg::ref_ptr<osg::Node> sceneData_p);

signals:
    void signal_parametersChanged(bool changed);
    void signal_loadRasterFromFile(QString filename_p = "");
    void signal_load3DSceneFromFile(QString filename_p = "");

};


#endif // USERFORMWIDGET_H
