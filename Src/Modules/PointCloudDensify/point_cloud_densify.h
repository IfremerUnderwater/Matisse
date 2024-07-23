#ifndef MATISSE_POINT_CLOUD_DENSIFY_H_
#define MATISSE_POINT_CLOUD_DENSIFY_H_


#include "processor.h"

namespace matisse {

/**
 * PointCloudDensify
 * @brief  This module get sparse point cloud and densify it using openMVS librarie
 */
class PointCloudDensify : public matisse::Processor
{
    Q_OBJECT
    Q_INTERFACES(matisse::Processor)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    Q_PLUGIN_METADATA(IID "PointCloudDensify")
#endif

public:
    PointCloudDensify();
    ~PointCloudDensify();
    virtual bool configure();
    virtual bool start();
    virtual bool stop();
    virtual void onFlush(quint32 _port);
    virtual void onNewImage(quint32 _port, matisse_image::Image &_image);


private:
    QString m_source_dir;
    QString m_outdir;
    QString m_out_filename_prefix;


    bool initDensify();
    bool DensifyPointCloud(QString _scene_dir, QString _scene_file);
};

} // namespace matisse

#endif // MATISSE_POINT_CLOUD_DENSIFY_H_
