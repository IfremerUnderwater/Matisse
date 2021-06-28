#if _MSC_VER
#define _USE_MATH_DEFINES
#include <math.h>
#endif

#include "draw_blend_2dmosaic.h"
#include "file_utils.h"
#include "geo_transform.h"
#include "mosaic_descriptor.h"
#include "mosaic_drawer.h"
#include "nav_image.h"
#include "raster_georeferencer.h"

using namespace cv;
using namespace system_tools;
using namespace optical_mapping;

// Export de la classe DrawAndWriteModule dans la bibliotheque de plugin DrawAndWriteModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(DrawBlend2DMosaic, DrawBlend2DMosaic)
#endif

namespace matisse {

DrawBlend2DMosaic::DrawBlend2DMosaic() :
    OutputDataWriter(NULL, "DrawBlend2DMosaic", "DrawBlend2DMosaic", 1)
{
    qDebug() << logPrefix() << "DrawBlend2DMosaic in constructor...";

    addExpectedParameter("dataset_param",  "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param",  "output_filename");

    addExpectedParameter("algo_param", "block_drawing");
    addExpectedParameter("algo_param", "block_width");
    addExpectedParameter("algo_param", "block_height");

    addExpectedParameter("algo_param", "disjoint_drawing");
    addExpectedParameter("algo_param", "single_image_output");

}

DrawBlend2DMosaic::~DrawBlend2DMosaic(){

}

bool DrawBlend2DMosaic::configure()
{
    qDebug() << logPrefix() << "configure";

    QString dataset_dirname_str = m_matisse_parameters->getStringParamValue("dataset_param", "dataset_dir");
    m_output_dirname_str = m_matisse_parameters->getStringParamValue("dataset_param", "output_dir");
    /* Resolve UNIX paths ('~/...') for remote job execution */
    m_output_dirname_str = FileUtils::resolveUnixPath(m_output_dirname_str);

    QString output_filename = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    if (dataset_dirname_str.isEmpty()
     || m_output_dirname_str.isEmpty()
     || output_filename.isEmpty())
        return false;


    QFileInfo output_dir_info(m_output_dirname_str);
    QFileInfo dataset_dir_info(dataset_dirname_str);

    bool is_relative_dir = output_dir_info.isRelative();

    if (is_relative_dir) {
        m_output_dirname_str = QDir::cleanPath( dataset_dir_info.absoluteFilePath() + QDir::separator() + m_output_dirname_str);
    }
    m_rasters_info.clear();

    return true;
}

void DrawBlend2DMosaic::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_port);
    Q_UNUSED(_image);
}

QList<QFileInfo> DrawBlend2DMosaic::rastersInfo()
{
    return m_rasters_info;
}

bool DrawBlend2DMosaic::start()
{
    return true;
}

bool DrawBlend2DMosaic::stop()
{
    return true;
}

void DrawBlend2DMosaic::onFlush(quint32 _port)
{

    emit si_processCompletion(0);
    emit si_userInformation("Drawing and blending 2D mosaic...");

    MosaicDescriptor *p_mosaic_d = NULL;
    //QVector<ProjectiveCamera*> *pCams = NULL;

    // Get pCams from mosaic _context
    /*QVariant * pCamsStocker = _context->getObject("Cameras");
    if (pCamsStocker) {
        pCams = pCamsStocker->value< QVector<ProjectiveCamera*>* >();
        qDebug()<< logPrefix() << "Receiving Cameras on port : " << port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << port;
    }*/

    // Get pMosaicD from mosaic _context
    QVariant * p_mosaic_d_stocker = m_context->getObject("MosaicDescriptor");
    if (p_mosaic_d_stocker) {
        p_mosaic_d = p_mosaic_d_stocker->value<MosaicDescriptor *>();
        qDebug()<< logPrefix() << "Receiving MosaicDescriptor on port : " << _port;
    }else{
        qDebug()<< logPrefix() << "No data to retreive on port : " << _port;
    }

    // Get drawing parameters
    bool ok;
    bool block_draw = m_matisse_parameters->getBoolParamValue("algo_param", "block_drawing", ok);
    qDebug() << logPrefix() << "block_drawing = " << block_draw;

    int block_width = m_matisse_parameters->getIntParamValue("algo_param", "block_width", ok);
    qDebug() << logPrefix() << "block_width = " << block_width;

    int block_height = m_matisse_parameters->getIntParamValue("algo_param", "block_height", ok);
    qDebug() << logPrefix() << "block_height = " << block_height;

    // Get drawing prefix
    QString output_filename = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    emit si_processCompletion(10);

    //Draw mosaic
    MosaicDrawer mosaic_drawer;
    QFileInfo output_file_info;

    QString output_type_choice = m_matisse_parameters->getStringParamValue("algo_param", "single_image_output");

    bool draw_geotiff = true;
    bool draw_jpeg = false;
    if (output_type_choice == "Geotiff only")
    {
        draw_geotiff = true;
        draw_jpeg = false;
    }
    if (output_type_choice == "JPEG only")
    {
        draw_geotiff = false;
        draw_jpeg = true;
    }
    if (output_type_choice == "Geotiff and JPEG")
    {
        draw_geotiff = true;
        draw_jpeg = true;
    }


    if (m_matisse_parameters->getBoolParamValue("algo_param", "disjoint_drawing", ok)) {
        QStringList output_files;
        
        if (draw_geotiff)
        {
            output_files = mosaic_drawer.writeImagesAsGeoTiff(*p_mosaic_d,
                m_output_dirname_str, output_filename);
        }

        if (draw_jpeg)
        {
            mosaic_drawer.outputMosaicImagesAsIs(*p_mosaic_d,
                m_output_dirname_str, output_filename);
        }

        foreach(QString filename, output_files) {
            output_file_info.setFile(QDir(m_output_dirname_str), filename);
            m_rasters_info << output_file_info;
        }

    }
    else if (!block_draw){
        // opencv331
        cv::UMat mosaic_image, mosaic_mask;
        mosaic_drawer.drawAndBlend(*p_mosaic_d, mosaic_image, mosaic_mask);

        emit si_processCompletion(50);

        // copy mask to force data pointer allocation in the right order
        Mat mask_copy;
        mosaic_mask.copyTo(mask_copy);
        mosaic_mask.release();
        Mat mat_mosaic_image = mosaic_image.getMat(ACCESS_READ);
        // Write geofile
        p_mosaic_d->writeToGeoTiff(mat_mosaic_image,mask_copy,m_output_dirname_str + QDir::separator() + output_filename + ".tiff");

        output_file_info.setFile(QDir(m_output_dirname_str), output_filename+ ".tiff");
        m_rasters_info << output_file_info;

    }else{
        qDebug()<< logPrefix() << "entered block drawing part...";
       
        
        QStringList output_files = mosaic_drawer.blockDrawBlendAndWrite(*p_mosaic_d,
                                                                      Point2d(block_width, block_height),
                                                                      m_output_dirname_str, output_filename);
        foreach (QString filename, output_files) {
            output_file_info.setFile(QDir(m_output_dirname_str), filename);
            m_rasters_info << output_file_info;
        }

    }

    emit si_processCompletion(100);
}

} // namespace matisse


