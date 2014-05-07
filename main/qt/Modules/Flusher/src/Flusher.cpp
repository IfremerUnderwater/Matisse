#include <QDir>

#include "Flusher.h"
Q_EXPORT_PLUGIN2(Flusher, Flusher)

Flusher::Flusher(QObject *parent):
    ImageProvider(NULL, "Flusher", "", 1),
    _pictureFileSet(NULL),
    _dim2FileReader(NULL)
{
    _imageSet = new ImageSet();

}

Flusher::~Flusher()
{

}



ImageSet * Flusher::imageSet(quint16 port)
{
    return _imageSet;
}

bool Flusher::configure(Context *context, MatisseParameters *mosaicParameters)
{
    qDebug() << logPrefix() << "Flusher configure";

    return true;

}

void Flusher::start()
{

    _imageSet->flush();

    qDebug() << logPrefix() << " out start";
}

void Flusher::stop()
{

    _imageSet->clear();
}

