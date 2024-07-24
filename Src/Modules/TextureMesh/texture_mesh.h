#pragma once

#include "processor.h"

namespace matisse {

/**
 * TextureMesh
 * @brief  This module texture a Mesh using photos
 */
class TextureMesh : public matisse::Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "TextureMesh")
#endif

public:
    TextureMesh();
    ~TextureMesh();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);


private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;

    bool initTexturing();
    bool textureMesh(QString& _mvs_data_file, QString& _mesh_filepath, QString& _output_textured_file);
};

} // namespace matisse
