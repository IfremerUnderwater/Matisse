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

    explicit InputDataProvider(QObject *_parent, QString m_name, QString _comment, quint16 _out_number);
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
    quint16 outNumber() {return m_out_number;}



    //
    // Methodes à surcharger - DEBUT
    //
    ///
    /// \brief Retourne l'ImageSet associé à un port de sortie
    /// \param _out_port
    /// \return
    ///
    virtual ImageSet * imageSet(quint16 _out_port) = 0;
    //
    // Methodes à surcharger - FIN
    //

    void callProcessingChain();

signals:
    void si_userInformation(QString _user_text);
    void si_processCompletion(quint8 _percent_complete);
    void si_showInformationMessage(QString _title, QString _text);
    void si_showErrorMessage(QString _title, QString _text);
    void si_show3DFileOnMainView(QString _filepath_p);
    void si_autoAdd3DFileFromFolderOnMainView(QString _folderpath_p);
    void si_addRasterFileToMap(QString _filepath_p);
    void si_addToLog(QString _loggin_text);

protected:
    ImageSet * m_image_set;

private:
    QString m_comment;
    quint16 m_out_number;

};

} // namespace matisse

Q_DECLARE_INTERFACE(matisse::InputDataProvider, "Ifremer.InputDataProvider/1.1")
#endif // MATISSE_INPUT_DATA_PROVIDER_H_
