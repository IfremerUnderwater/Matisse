#ifndef IMAGELISTENER_H
#define IMAGELISTENER_H

#include <QObject>
#include "Image.h"

namespace MatisseCommon {

///
/// \brief Classe de notification d'arrivée de nouvelle image
///
class ImageListener : public QObject
{
    Q_OBJECT
public:
    explicit ImageListener(QObject *parent = 0);

    ///
    /// \brief Notifie l'arrivée d'une nouvelle image sur le port d'entrée
    /// \param port
    /// \param image
    ///
    virtual void onNewImage(quint32 port, Image &image) = 0;

    virtual void onFlush(quint32 port) {}
signals:
    
public slots:
};

}

#endif // IMAGELISTENER_H
