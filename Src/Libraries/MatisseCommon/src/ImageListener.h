#ifndef IMAGELISTENER_H
#define IMAGELISTENER_H

#include "libmatissecommon_global.h"

#include <QObject>
#include "Image.h"

namespace MatisseCommon {

///
/// \brief Interface de notification d'arrivée de nouvelle image
///
class LIBMATISSECOMMONSHARED_EXPORT ImageListener
{
public:
    virtual ~ImageListener() {}
    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param port
    /// \param image
    ///
    virtual void onNewImage(quint32 port, Image &image) = 0;

    virtual void onFlush(quint32 port) = 0;


};

}
Q_DECLARE_INTERFACE(MatisseCommon::ImageListener, "Chrisar.ImageListener/1.1")
#endif // IMAGELISTENER_H
