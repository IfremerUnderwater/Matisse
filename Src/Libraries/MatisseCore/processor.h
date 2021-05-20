#ifndef PROCESSOR_H
#define PROCESSOR_H



#include <QList>
#include <QtPlugin>

#include "image_listener.h"
#include "Context.h"
#include "image.h"
#include "image_set.h"
#include "MatisseParameters.h"
#include "LifecycleComponent.h"
#include "Polygon.h"

namespace MatisseCommon {

///
/// \brief Classe de base des modules chargeables dans MatisseServer
///
class Processor : public QObject, public ImageListener, public LifecycleComponent
{
    Q_OBJECT
    Q_INTERFACES(MatisseCommon::ImageListener)
    Q_INTERFACES(MatisseCommon::LifecycleComponent)
public:

    
    // Accesseurs
    QString comment(){return _comment;}
    quint16 inNumber() {return _inNumber;}
    quint16 outNumber() {return _outNumber;}

    bool setInputPortList(QList<ImageSetPort *> * inputPortList);
    bool setOutputPortList(QList<ImageSetPort *> * outputPortList);

    bool okStatus() { return _okStatus; }

    // Must be called in start
    // and eventually in flush or onNewImage
    void setOkStatus(const bool status = true)
    {
        _okStatus = status;
    }

protected:
    ///
    /// \brief Processor
    /// \param parent Pour la composition Qt. Peut être NULL
    /// \param name Nom unique du composant dans MatisseServer
    /// \param comment Commentaire lié au composant
    /// \param inNumber Nombre de ports d'entrée
    /// \param outNumber Nombre de ports de sortie
    ///
    explicit Processor(QObject *parent, QString name, QString comment, quint16 inNumber, quint16 outNumber);
    virtual ~Processor() {}

    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param port
    /// \param image
    ///
    virtual void onNewImage(quint32 port, Image &image) = 0;

    ///
    /// \brief Emission d'une image sur un port de sortie.
    ///
    /// Doit être appelé pour émettre le résultat d'un traitement.
    /// \param port port de sortie
    /// \param image Image à envoyer
    ///
    void postImage(quint32 port, Image &image);

    ///
    /// \brief flush Traiter un lot d'images
    /// \param port port de sortie
    ///
    void flush(quint32 port);

signals:
    void signal_showImageOnMainView(Image* image);
    void signal_userInformation(QString userText);
    void signal_processCompletion(quint8 percentComplete);
    void signal_showInformationMessage(QString title, QString text);
    void signal_showErrorMessage(QString title, QString text);
    void signal_fatalError();
    void signal_show3DFileOnMainView(QString filepath_p);
    void signal_addRasterFileToMap(QString filepath_p);
    void signal_addToLog(QString _loggin_text);

private:
    QString _comment;
    quint16 _inNumber;
    quint16 _outNumber;

protected:
    QList<ImageSetPort *> * _inputPortList;
    QList<ImageSetPort *> * _outputPortList;

    volatile bool _okStatus;

    void fatalErrorExit(QString message);
};
}
Q_DECLARE_INTERFACE(MatisseCommon::Processor, "Ifremer.Processor/1.1")

#endif // PROCESSOR_H
