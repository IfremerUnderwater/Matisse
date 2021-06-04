#ifndef MATISSE_IMAGE_IMAGE_SET_H_
#define MATISSE_IMAGE_IMAGE_SET_H_


#include <QObject>
#include <QList>

#include "image.h"
#include "image_listener.h"

namespace matisse_image {

class ImageSet;

///
/// \brief Definition d'un port de communication sortant
///
struct ImageSetPort {
    quint16 port_number;
    ImageSet * image_set;
    ImageListener* image_listener;
};

///
/// \brief Connexion entre des ports de communication.
///
/// Un ImageSet permet d'échanger des images entre une instance d'ImageProvider et une instance de Processor ou entre plusieurs instances de Processor.
///
class ImageSet : public QObject
{

    Q_OBJECT
public:
    ///
    /// \brief Construit un ImageSet
    /// \param _max_size Nombre maximum d'image dans l'imageset. -1 si pas de limite
    /// \param _drop_images Lorsque le nombre d'image maximum est atteint si dropImage est vrai, les images les plus anciennes sont supprimées. Si dropImage est faux, la nouvelle image n'est pas ajoutée
    /// \param _parent
    ///
    ImageSet(int _max_size = -1, bool _drop_images = true, QObject * _parent = 0):QObject(_parent),
        m_max_size(_max_size),
        m_drop_images(_drop_images),
        m_out_port(NULL)
    {

    }

public:
    ///
    /// \brief Retourne l'image associée à un index dans la liste d'image
    /// \param _image_number
    /// \return
    ///
    Image * getImage(int _image_number = 0);

    ///
    /// \brief Retourne la liste de toutes les images
    /// \return
    ///
    QList<Image *> getAllImages();


    ///
    /// \brief Retourne un sous ensemble de la liste de toutes les images
    /// \param _size nombre maximum du sous-ensemble
    /// \param _image_number image de départ du sous-ensemble
    /// \return
    ///
    QList<Image *> getImages(int _size = 50, int _image_number = 0);

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
    /// \param _image
    /// \return faux si l'image n'a pas pu être ajoutée.
    ///
    bool addImage(Image * _image);

    ///
    /// \brief Ajoute une liste d'image dans l'ImageSet.
    ///
    /// Voir  \ref addImage(Image *).
    /// \param _images
    /// \return
    ///
    bool addImages(QList<Image *> m_images);


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
    /// \param _out_port
    ///
    void setOutPort(ImageSetPort *_out_port);

private:
    QList<Image *> m_images;
    int m_max_size;
    bool m_drop_images;
    ImageSetPort *m_out_port;
};
}

#endif // MATISSE_IMAGE_IMAGE_SET_H_
