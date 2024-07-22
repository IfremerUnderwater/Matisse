#pragma once

#include <QFileSystemWatcher>
#include <QDateTime>
#include "processor.h"

namespace matisse {

/**
 * Module1
 * @brief  Exemple de module pour implementer un algorithme de traitement dans Matisse
 */
class ColmapMapper : public Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ColmapMapper")
#endif

public:
    ColmapMapper();
    ~ColmapMapper();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);

private:

    bool m_use_prior;

    bool sfmMapper();

    bool undistortImages(QString &_image_path, QString &_sfmdir, QString &_outdir);

};

} // namespace matisse
