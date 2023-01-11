#ifndef MATISSE_PROCESSOR_H_
#define MATISSE_PROCESSOR_H_


#include <QList>
#include <QtPlugin>

#include "image_listener.h"
#include "Context.h"
#include "image.h"
#include "image_set.h"
#include "matisse_parameters.h"
#include "lifecycle_component.h"
#include "Polygon.h"

using namespace matisse_image;

namespace matisse {

///
/// \brief Classe de base des modules chargeables dans MatisseServer
///
class Processor : public QObject, public matisse_image::ImageListener, public LifecycleComponent
{
    Q_OBJECT
    Q_INTERFACES(matisse_image::ImageListener)
    Q_INTERFACES(matisse::LifecycleComponent)
public:

    
    // Accesseurs
    QString comment(){return m_comment;}
    quint16 inNumber() {return m_in_number;}
    quint16 outNumber() {return m_out_number;}

    bool setInputPortList(QList<matisse_image::ImageSetPort *> * _input_port_list);
    bool setOutputPortList(QList<matisse_image::ImageSetPort *> * _output_port_list);

    bool okStatus() { return m_ok_status; }

    // Must be called in start
    // and eventually in flush or onNewImage
    void setOkStatus(const bool _status = true)
    {
        m_ok_status = _status;
    }

protected:
    ///
    /// \brief Processor
    /// \param _parent Pour la composition Qt. Peut être NULL
    /// \param _name Nom unique du composant dans MatisseServer
    /// \param _comment Commentaire lié au composant
    /// \param _in_number Nombre de ports d'entrée
    /// \param _out_number Nombre de ports de sortie
    ///
    explicit Processor(QObject *_parent, QString _name, QString _comment, quint16 _in_number, quint16 _out_number);
    virtual ~Processor() {}

    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param _port
    /// \param _image
    ///
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image) = 0;

    ///
    /// \brief Emission d'une image sur un port de sortie.
    ///
    /// Doit être appelé pour émettre le résultat d'un traitement.
    /// \param _port port de sortie
    /// \param _image Image à envoyer
    ///
    void postImage(quint32 _port, matisse_image::Image &_image);

    virtual matisse_image::ImageSet * nextImageSet() override;

    ///
    /// \brief flush Traiter un lot d'images
    /// \param _port port de sortie
    ///
//    void flush(quint32 _port);

signals:
    void si_showImageOnMainView(matisse_image::Image* _image);
    void si_userInformation(QString _user_text);
    void si_processCompletion(quint8 _percent_complete);
    void si_showInformationMessage(QString _title, QString _text);
    void si_showErrorMessage(QString _title, QString _text);
    void si_fatalError();
    void si_show3DFileOnMainView(QString _filepath_p);
    void si_autoAdd3DFileFromFolderOnMainView(QString _folderpath_p);
    void si_addRasterFileToMap(QString _filepath_p);
    void si_addToLog(QString _loggin_text);

private:
    QString m_comment;
    quint16 m_in_number;
    quint16 m_out_number;

protected:
    QList<matisse_image::ImageSetPort *> * m_input_port_list;
    QList<matisse_image::ImageSetPort *> * m_output_port_list;

    volatile bool m_ok_status;

    void fatalErrorExit(QString _message);
};

} // namespace matisse

Q_DECLARE_INTERFACE(matisse::Processor, "Ifremer.Processor/1.1")

#endif // MATISSE_PROCESSOR_H_
