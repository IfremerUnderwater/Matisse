#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QObject>

#include "ImageSet.h"
#include "Context.h"
#include "MatisseParameters.h"

namespace MatisseCommon {
///
/// \brief Classe de base des fournisseurs d'image
/// Un fournisseur d'image possède plusieurs port de sortie d'image. Les ports de sortie sont connectés aux ports d'entrées des Processors par assemblage.
/// Une instance d'ImageProvider est construite au démarrage de Matisse.
/// Les methodes de cycle de vie virtuelles pures \ref configure(Context * , MatisseParameters *)\ref start() \ref stop() doivent être implémentées par les classes dérivées.
/// Elles sont appelées par le moteur à chaque exécution d'un assemblage.
///
class ImageProvider : public QObject
{
    Q_OBJECT
public:

    explicit ImageProvider(QObject *parent, QString name, QString comment, quint16 outNumber);
    virtual ~ImageProvider();

    ///
    /// \brief Retourne le nom unique spécifié dans le constructeur.
    /// \return
    ///
    QString name() const;

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

    ///
    /// \brief Appelle configure de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    /// \param context
    /// \param mosaicParameters
    ///
    bool callConfigure(Context * context, MatisseParameters * mosaicParameters);

    ///
    /// \brief Appelle start de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    void callStart();

    ///
    /// \brief Appelle stop de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    void callStop();


    ///
    /// \brief Retourne les le couple (structure,parametre) attendus dans MatisseParameters. Utilisé par le moteur d'assemblage.
    /// \return
    ///
    QList<MatisseParameter> expectedParameters() { return _expectedParameters; }

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

signals:
    
public slots:

protected:

    //
    // Methodes à surcharger - DEBUT
    //

    ///
    /// \brief Configuration du processeur pour l'execution d'un assemblage
    ///
    /// \param context Le contexte permet de passer des objets entre les différents processeurs.
    /// \param mosaicParameters Les paramètres sont les informations saisies par l'utilisateur en lecture seule.
    /// \return
    ///
    virtual bool configure(Context * context, MatisseParameters * mosaicParameters) = 0;

    ///
    /// \brief La méthode start est appelée pour prevenir du commencement de l'execution d'un asssemblage.
    ///
    virtual void start() = 0;

    ///
    /// \brief La méthode stop est appelée pour prevenir de la fin de l'execution d'un asssemblage.
    ///
    virtual void stop() = 0;


    //
    // Methodes à surcharger - FIN
    //

    ///
    /// \brief Ajoute un couple (structure,parametre) aux paramètres attendus. Doit être appelé dans le constructeur de la classe dérivée.
    /// \param structure
    /// \param param
    ///
    void addExpectedParameter(QString structure, QString param);

    ///
    /// \brief Retourne un chaine pour le déboggage
    /// \return
    ///
    QString const & logPrefix() const;

private:
    QString _name;
    QString _comment;
    quint16 _outNumber;

    QList<MatisseParameter> _expectedParameters;
    QString _logPrefix;
};
}
Q_DECLARE_INTERFACE(MatisseCommon::ImageProvider, "Chrisar.ImageProvider/1.0")
#endif // IMAGEPROVIDER_H
