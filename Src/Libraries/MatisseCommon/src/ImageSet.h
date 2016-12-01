#ifndef IMAGESET_H
#define IMAGESET_H

#include "libmatissecommon_global.h"

#include <QObject>
#include <QList>

#include "Image.h"
#include "ImageListener.h"

namespace MatisseCommon {
class ImageSet;

///
/// \brief Definition d'un port de communication sortant
///
struct ImageSetPort {
    quint16 portNumber;
    ImageSet * imageSet;
    ImageListener* imageListener;
};

///
/// \brief Connexion entre des ports de communication.
///
/// Un ImageSet permet d'échanger des images entre une instance d'ImageProvider et une instance de Processor ou entre plusieurs instances de Processor.
///
class LIBMATISSECOMMONSHARED_EXPORT ImageSet : public QObject
{

    Q_OBJECT
public:
    ///
    /// \brief Construit un ImageSet
    /// \param maxSize Nombre maximum d'image dans l'imageset. -1 si pas de limite
    /// \param dropImages Lorsque le nombre d'image maximum est atteint si dropImage est vrai, les images les plus anciennes sont supprimées. Si dropImage est faux, la nouvelle image n'est pas ajoutée
    /// \param parent
    ///
    ImageSet(int maxSize = -1, bool dropImages = true, QObject * parent = 0):QObject(parent),
        _maxSize(maxSize),
        _dropImages(dropImages),
        _outPort(NULL)
    {

    }

public:
    ///
    /// \brief Retourne l'image associée à un index dans la liste d'image
    /// \param imageNumber
    /// \return
    ///
    Image * getImage(int imageNumber = 0);

    ///
    /// \brief Retourne la liste de toutes les images
    /// \return
    ///
    QList<Image *> getAllImages();


    ///
    /// \brief Retourne un sous ensemble de la liste de toutes les images
    /// \param size nombre maximum du sous-ensemble
    /// \param imageNumber image de départ du sous-ensemble
    /// \return
    ///
    QList<Image *> getImages(int size = 50, int imageNumber = 0);

    ///
    /// \brief Retourne le nombre d'images
    /// \return
    ///
    int getNumberOfImages();

    ///
    /// \brief Retourne la taille maximum de l'ImageSet
    /// \return
    ///
    int getMaxSize();

    ///
    /// \brief Ajoute une image dans l'ImageSet.
    ///
    /// Selon l'attribut dropImages, les anciennes images peuvent être supprimée si la limite max d'image est atteinte.
    /// \param image
    /// \return faux si l'image n'a pas pu être ajoutée.
    ///
    bool addImage(Image * image);

    ///
    /// \brief Ajoute une liste d'image dans l'ImageSet.
    ///
    /// Voir  \ref addImage(Image *).
    /// \param images
    /// \return
    ///
    bool addImages(QList<Image *> images);


    ///
    /// \brief flush Traiter un lot d'images
    ///
    void flush();

    ///
    /// \brief Supprime toutes les images de l'ImageSet
    ///
    void clear();

    ///
    /// \brief Fonction de deboggage
    /// \return
    ///
    QString dumpAttr();

    ///
    /// \brief Retourne le port de connexion sortant associé à l'ImageSet
    ///
    /// Utilisé par le moteur d'assemblage.
    /// \return
    ///
    ImageSetPort *outPort() const;

    ///
    /// \brief Définit le port de connexion sortant associé à l'ImageSet
    ///
    /// Utilisé par le moteur d'assemblage.
    /// \param outPort
    ///
    void setOutPort(ImageSetPort *outPort);

private:
    QList<Image *> _images;
    int _maxSize;
    bool _dropImages;
    ImageSetPort *_outPort;
};
}

#endif // IMAGESET_H
