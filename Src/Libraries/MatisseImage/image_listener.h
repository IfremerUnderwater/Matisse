#ifndef MATISSE_IMAGE_IMAGE_LISTENER_H_
#define MATISSE_IMAGE_IMAGE_LISTENER_H_


#include <QObject>
#include "image.h"

namespace matisse_image {

///
/// \brief Interface de notification d'arrivée de nouvelle image
///
class ImageListener
{
public:
    virtual ~ImageListener() {}
    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param _port
    /// \param _image
    ///
    virtual void onNewImage(quint32 _port, Image &_image) = 0;

    virtual void onFlush(quint32 _port) = 0;


};

}
Q_DECLARE_INTERFACE(matisse_image::ImageListener, "Ifremer.ImageListener/1.1")
#endif // MATISSE_IMAGE_IMAGE_LISTENER_H_
