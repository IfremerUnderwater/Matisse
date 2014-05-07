#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <QList>
#include <QtPlugin>

#include "ImageListener.h"
#include "Context.h"
#include "Image.h"
#include "ImageSet.h"
#include "MatisseParameters.h"



namespace MatisseCommon {

///
/// \brief Classe de base des modules chargeables dans MatisseServer
///
class Processor : public ImageListener
{
    Q_OBJECT
public:

    
    // Accesseurs
    QString name(){return _name;}
    QString comment(){return _comment;}
    quint16 inNumber() {return _inNumber;}
    quint16 outNumber() {return _outNumber;}

    ///
    /// \brief Retourne les le couple (structure,parametre) attendus dans MatisseParameters. Utilisé par le moteur d'assemblage.
    /// \return
    ///
    QList<MatisseParameter> expectedParameters();

    bool setInputPortList(QList<ImageSetPort *> * inputPortList);
    bool setOutputPortList(QList<ImageSetPort *> * outputPortList);

    ///
    /// \brief Appelle configure de la classe dérivée
    ///
    /// Methode appelée par le moteur d'assemblage
    /// \param context
    /// \param mosaicParameters
    ///
    void callConfigure(Context * context, MatisseParameters * mosaicParameters);

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

    //
    // Methodes à surcharger - DEBUT
    //




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
    /// \brief Configuration du processeur pour l'execution d'un assemblage
    ///
    /// \param context Le contexte permet de passer des objets entre les différents processeurs.
    /// \param mosaicParameters Les paramètres sont les informations saisies par l'utilisateur en lecture seule.
    /// \return
    ///
    virtual void configure(Context * context, MatisseParameters * mosaicParameters)=0;

    ///
    /// \brief La méthode start est appelée pour prevenir du commencement de l'execution d'un asssemblage.
    ///
    virtual void start() = 0;

    ///
    /// \brief La méthode stop est appelée pour prevenir de la fin de l'execution d'un asssemblage.
    ///
    virtual void stop() = 0;


    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param port
    /// \param image
    ///
    virtual void onNewImage(quint32 port, Image &image) = 0;

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

    ///
    /// \brief Retourne un chaine pour le déboggage
    /// \return
    ///
    QString const & logPrefix() const;


private:
    QString _name;
    QString _comment;
    quint16 _inNumber;
    quint16 _outNumber;
    QString _logPrefix;


    QList<MatisseParameter> _expectedParameters;

protected:
    // TODO Stocker les port par numéro?
    QList<ImageSetPort *> * _inputPortList;
    QList<ImageSetPort *> * _outputPortList;
    Context * _context;
    MatisseParameters * _mosaicParameters;

signals:
    
public slots:
    
};
}
Q_DECLARE_INTERFACE(MatisseCommon::Processor, "Chrisar.Processor/1.0")

#endif // PROCESSOR_H
