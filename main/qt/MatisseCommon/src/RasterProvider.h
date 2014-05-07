#ifndef RASTERPROVIDER_H
#define RASTERPROVIDER_H

#include "Context.h"
#include "ImageListener.h"
#include "Image.h"
#include "ImageSet.h"
#include "MatisseParameters.h"

namespace MatisseCommon {

class RasterProvider : public ImageListener {

    Q_OBJECT
public:

    // Accesseurs
    QString name(){return _name;}
    QString comment(){return _comment;}
    quint16 inNumber() {return _inNumber;}


    explicit RasterProvider(QObject *parent, QString name, QString comment, quint16 inNumber =1);
    virtual ~RasterProvider();

    ///
    /// \brief Retourne les le couple (structure,parametre) attendus dans MatisseParameters. Utilisé par le moteur d'assemblage.
    /// \return
    ///
    QList<MatisseParameter> expectedParameters() { return _expectedParameters; }

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

    ImageSet *imageSet() const;
    void setImageSet(ImageSet *imageSet);

protected:
    ///
    /// \brief Configuration du processeur pour l'execution d'un assemblage
    ///
    /// \param context Le contexte permet de passer des objets entre les différents processeurs.
    /// \param mosaicParameters Les paramètres sont les informations saisies par l'utilisateur en lecture seule.
    /// \return
    ///
    virtual bool configure(Context * context, MatisseParameters * mosaicParameters)=0;

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

    ///
    /// \brief Ajoute un couple (structure,parametre) aux paramètres attendus. Doit être appelé dans le constructeur de la classe dérivée.
    /// \param structure
    /// \param param
    ///
    void addExpectedParameter(QString structure, QString param);


public:
    virtual QFileInfo rasterInfo() = 0;
    //
    // Methodes à surcharger - FIN
    //
protected:
    ///
    /// \brief Retourne un chaine pour le déboggage
    /// \return
    ///
    QString const & logPrefix() const;

private:
    QString _name;
    QString _comment;
    quint16 _inNumber;
    QString _logPrefix;

    ImageSet *_imageSet;
    QList<MatisseParameter> _expectedParameters;

protected:
    Context * _context;
    MatisseParameters * _mosaicParameters;
};


}
Q_DECLARE_INTERFACE(MatisseCommon::RasterProvider, "Chrisar.RasterProvider/1.0")

#endif // RASTERPROVIDER_H
