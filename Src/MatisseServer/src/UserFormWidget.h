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
    void signal_add3DSceneToCartoView(QString userText);

public slots:
    void slot_loadRasterFromFile(QString filename_p = "");
    void slot_load3DSceneFromFile(QString filename = "");

private:
    CartoViewType _lastLoadedView;


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
    void load3DFile(QString filename = "");
    void loadImageFile(QString filename);
    void setTools(Tools * tools);
    void saveQgisProject(QString filename);
    void loadTestVectorLayer();
    
    CartoViewType currentViewType() const;

    QStringList supportedRasterFormat() const;
    QStringList supportedVectorFormat() const;
    QStringList supported3DFileFormat() const;
    QStringList supportedImageFormat() const;

private:
    Ui::UserFormWidget *_ui;
    Tools * _tools;
    ParametersWidgetSkeleton * _parametersWidget;
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

signals:
    void signal_parametersChanged(bool changed);
    void signal_loadRasterFromFile(QString filename_p = "");

};


#endif // USERFORMWIDGET_H
