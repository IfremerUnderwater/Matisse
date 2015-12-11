#include <QDir>

#include "Flusher.h"
Q_EXPORT_PLUGIN2(Flusher, Flusher)

Flusher::Flusher(QObject *parent):
    ImageProvider(NULL, "Flusher", "", 1),
    _pictureFileSet(NULL),
    _dim2FileReader(NULL)
{
    Q_UNUSED(parent);
    _imageSet = new ImageSet();

}

Flusher::~Flusher()
{

}



ImageSet * Flusher::imageSet(quint16 port)
{
    Q_UNUSED(port);
    return _imageSet;
}

bool Flusher::configure()
{
    qDebug() << logPrefix() << "Flusher configure";

    return true;

}

bool Flusher::start()
{
    _imageSet->flush();
    qDebug() << logPrefix() << " out start";
    return true;
}

bool Flusher::stop()
{
    _imageSet->clear();
    return true;
}

