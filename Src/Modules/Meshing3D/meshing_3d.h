#ifndef MATISSE_MESHING_3D_H_
#define MATISSE_MESHING_3D_H_


#include "processor.h"

namespace matisse {

/**
 * Meshing3D
 * @brief  This module create a 3D Mesh from sparse of dense 3D point cloud
 */
class Meshing3D : public matisse::Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "Meshing3D")
#endif

public:
    Meshing3D();
    ~Meshing3D();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);


private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    bool initMeshing();
    bool meshing(QString _mvs_data_file);
};

} // namespace matisse

#endif // MATISSE_MESHING_3D_H_
