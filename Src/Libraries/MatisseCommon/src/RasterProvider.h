#ifndef RASTERPROVIDER_H
#define RASTERPROVIDER_H

#include "LifecycleComponent.h"
#include "ImageListener.h"
#include "Image.h"
#include "ImageSet.h"
#include "MatisseParameters.h"
#include "Polygon.h"

namespace MatisseCommon {

class RasterProvider : public QObject, public ImageListener, public LifecycleComponent  {
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageListener)
    Q_INTERFACES(MatisseCommon::LifecycleComponent)
public:

    // Accesseurs
    QString comment(){return _comment;}
    quint16 inNumber() {return _inNumber;}


    explicit RasterProvider(QObject *parent, QString name, QString comment, quint16 inNumber =1);
    virtual ~RasterProvider();

    ImageSet *imageSet() const;
    void setImageSet(ImageSet *imageSet);

public:
    virtual QList<QFileInfo> rastersInfo() = 0;

signals:
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_show3DFileOnMainView(QString filepath_p);
    void signal_addPolygonToMap(basicproc::Polygon polygon_p, QString polyInsideColor_p, QString layerName_p);

private:
    QString _comment;
    quint16 _inNumber;
    ImageSet *_imageSet;


};


}
Q_DECLARE_INTERFACE(MatisseCommon::RasterProvider, "Chrisar.RasterProvider/1.1")

#endif // RASTERPROVIDER_H
