#ifndef RASTERPROVIDER_H
#define RASTERPROVIDER_H

#include "lifecycle_component.h"
#include "image_listener.h"
#include "image.h"
#include "image_set.h"
#include "matisse_parameters.h"
#include "Polygon.h"

using namespace matisse_image;

namespace MatisseCommon {

class OutputDataWriter : public QObject, public ImageListener, public LifecycleComponent  {
    Q_OBJECT
    Q_INTERFACES(matisse_image::ImageListener)
    Q_INTERFACES(MatisseCommon::LifecycleComponent)
public:

    // Accesseurs
    QString comment(){return _comment;}
    quint16 inNumber() {return _inNumber;}


    explicit OutputDataWriter(QObject *parent, QString name, QString comment, quint16 inNumber =1);
    virtual ~OutputDataWriter();

    ImageSet *imageSet() const;
    void setImageSet(ImageSet *imageSet);

public:
    virtual QList<QFileInfo> rastersInfo() = 0;

signals:
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_showInformationMessage(QString title, QString text);
    void signal_showErrorMessage(QString title, QString text);
    void signal_show3DFileOnMainView(QString filepath_p);
    void signal_addRasterFileToMap(QString filepath_p);
    void signal_addToLog(QString _loggin_text);

private:
    QString _comment;
    quint16 _inNumber;
    ImageSet *_imageSet;


};


}
Q_DECLARE_INTERFACE(MatisseCommon::OutputDataWriter, "Ifremer.OutputDataWriter/1.1")

#endif // RASTERPROVIDER_H
