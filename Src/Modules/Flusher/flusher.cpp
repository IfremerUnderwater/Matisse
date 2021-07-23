#include <QDir>

#include "flusher.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Flusher, Flusher)
#endif

namespace matisse {

Flusher::Flusher(QObject *_parent):
    InputDataProvider(NULL, "Flusher", "Flusher", 1),
    m_picture_file_set(NULL),
    m_dim2_file_reader(NULL)
{
    Q_UNUSED(_parent);
    m_image_set = new ImageSet();

}

Flusher::~Flusher()
{

}


ImageSet * Flusher::imageSet(quint16 _port)
{
    Q_UNUSED(_port);
    return m_image_set;
}

bool Flusher::configure()
{
    qDebug() << logPrefix() << "Flusher configure";

    return true;

}

bool Flusher::start()
{    
    ImageSet *next_step = m_image_set;

    while (next_step) {
//        m_image_set->flush();
        next_step->flush();
        next_step = next_step->nextStep();
    }

    qDebug() << logPrefix() << " out start";
    return true;
}

bool Flusher::stop()
{
    m_image_set->clear();
    return true;
}

} // namespace matisse


