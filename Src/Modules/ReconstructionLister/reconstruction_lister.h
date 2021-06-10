#ifndef MATISSE_RECONSTRUCTION_LISTER_H_
#define MATISSE_RECONSTRUCTION_LISTER_H_


#include "output_data_writer.h"
#include "picture_file_set.h"
#include "image_set.h"
#include "file_image.h"

namespace matisse {

class ReconstructionLister : public OutputDataWriter
{
    Q_OBJECT
    Q_INTERFACES(matisse::OutputDataWriter)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "ReconstructionLister")
#endif

public:
    explicit ReconstructionLister(QObject *_parent = 0);
    virtual ~ReconstructionLister();

    virtual void onNewImage(quint32 _port, Image &_image);
    virtual void onFlush(quint32 _port);
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual QList<QFileInfo> rastersInfo();

private:
    QList<QFileInfo> m_rasters_info;

signals:
    
public slots:
    
};

} // namespace matisse

#endif // MATISSE_RECONSTRUCTION_LISTER_H_
