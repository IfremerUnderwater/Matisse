#ifndef MATISSE_INPUT_DATA_PROVIDER_H_
#define MATISSE_INPUT_DATA_PROVIDER_H_


#include <QObject>
#include "image_set.h"
#include "Context.h"
#include "lifecycle_component.h"
#include "Polygon.h"

using namespace matisse_image;

namespace matisse {

///
/// \brief Classe de base des fournisseurs d'image
/// Un fournisseur d'image possède plusieurs port de sortie d'image. Les ports de sortie sont connectés aux ports d'entrées des Processors par assemblage.
/// Une instance d'ImageProvider est construite au démarrage de Matisse.
/// Les methodes de cycle de vie virtuelles pures \ref configure(Context * , MatisseParameters *)\ref start() \ref stop() doivent être implémentées par les classes dérivées.
/// Elles sont appelées par le moteur à chaque exécution d'un assemblage.
///
class InputDataProvider : public QObject, public LifecycleComponent
{
    Q_OBJECT
    Q_INTERFACES(matisse::LifecycleComponent)
public:

    explicit InputDataProvider(QObject *parent, QString name, QString comment, quint16 outNumber);
    virtual ~InputDataProvider();



    ///
    /// \brief Retourne le commentaire associé au composant.
    /// \return
    ///
    QString comment() const;

    ///
    /// \brief Retourne le nombre de port de ce fournisseur.
    /// \return
    ///
    quint16 outNumber() {return _outNumber;}



    //
    // Methodes à surcharger - DEBUT
    //
    ///
    /// \brief Retourne l'ImageSet associé à un port de sortie
    /// \param port
    /// \return
    ///
    virtual ImageSet * imageSet(quint16 outPort) = 0;
    //
    // Methodes à surcharger - FIN
    //


    bool isRealTime() const;
protected:
    void setIsRealTime(bool isRealTime);

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
    quint16 _outNumber;
    bool _isRealTime;

};

} // namespace matisse

Q_DECLARE_INTERFACE(matisse::InputDataProvider, "Ifremer.InputDataProvider/1.1")
#endif // MATISSE_INPUT_DATA_PROVIDER_H_
