#ifndef MATISSE_OUTPUT_DATA_WRITER_H_
#define MATISSE_OUTPUT_DATA_WRITER_H_

#include "lifecycle_component.h"
#include "image_listener.h"
#include "image.h"
#include "image_set.h"
#include "matisse_parameters.h"
#include "Polygon.h"

using namespace matisse_image;

namespace matisse {

class OutputDataWriter : public QObject, public ImageListener, public LifecycleComponent  {
    Q_OBJECT
    Q_INTERFACES(matisse_image::ImageListener)
    Q_INTERFACES(matisse::LifecycleComponent)
public:

    // Accesseurs
    QString comment(){return m_comment;}
    quint16 inNumber() {return m_in_number;}


    explicit OutputDataWriter(QObject *_parent, QString _name, QString _comment, quint16 _in_number =1);
    virtual ~OutputDataWriter();

    ImageSet *imageSet() const;
    void setImageSet(ImageSet *_image_set);

public:
    virtual QList<QFileInfo> rastersInfo() = 0;
    virtual ImageListenerOutput * nextImageSet() override;

signals:
    void si_userInformation(QString _user_text);
    void si_processCompletion(quint8 _percent_complete);
    void si_showInformationMessage(QString _title, QString _text);
    void si_showErrorMessage(QString _title, QString _text);
    void si_show3DFileOnMainView(QString _filepath_p);
    void si_addRasterFileToMap(QString _filepath_p);
    void si_addToLog(QString _loggin_text);

private:
    QString m_comment;
    quint16 m_in_number;
    ImageSet *m_image_set;


};

} // namespace matisse

Q_DECLARE_INTERFACE(matisse::OutputDataWriter, "Ifremer.OutputDataWriter/1.1")

#endif // MATISSE_OUTPUT_DATA_WRITER_H_
