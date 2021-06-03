#include "point_cloud_densify.h"
#include "reconstruction_context.h"
#include "nav_image.h"

#define OPENMVG_USE_OPENMP
#include "mvg_mvs_interface.h"

#include <QProcess>
#include <QElapsedTimer>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

// for openMVS
#define _USE_BOOST
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "OpenMVS/MVS/Common.h"
#include "OpenMVS/MVS/Scene.h"
#include "omp.h"
#include <QFileInfo>

using namespace MVS;

namespace OPT {
    int thFilterPointCloud;
    int nFusionMode;
    int nArchiveType;
    unsigned nMaxThreads;
} // namespace OPT

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::geometry;
using namespace openMVG::image;
using namespace openMVG::sfm;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(PointCloudDensify, PointCloudDensify)
#endif

namespace matisse {

PointCloudDensify::PointCloudDensify() :
    Processor(NULL, "PointCloudDensify", "Densify a 3D sparse point cloud", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "resolution_level");
    addExpectedParameter("algo_param", "number_views_fuse");
}

PointCloudDensify::~PointCloudDensify(){

}

bool PointCloudDensify::configure()
{
    return true;
}

void PointCloudDensify::onNewImage(quint32 port, matisse_image::Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);

}

bool PointCloudDensify::initDensify()
{
    // Default param init
    OPTDENSE::init();
    OPTDENSE::update();
    OPTDENSE::nResolutionLevel = 1;
    OPTDENSE::nMaxResolution = 3200;
    OPTDENSE::nMinResolution = 640;
    OPTDENSE::nNumViews = 5;
    OPTDENSE::nMinViewsFuse = 3;
    OPTDENSE::nOptimize = 7;
    OPTDENSE::nEstimateColors = 2;
    OPTDENSE::nEstimateNormals = 0;

    // Get user params
    bool ok = true;
    int reslevel = _matisseParameters->getIntParamValue("algo_param", "resolution_level", ok);
    if (ok)
        OPTDENSE::nResolutionLevel = reslevel;

    int nbviewfuse = _matisseParameters->getIntParamValue("algo_param", "number_views_fuse", ok);
    if (ok)
        OPTDENSE::nMinViewsFuse = nbviewfuse;

    // Set max threads
    OPT::nMaxThreads = omp_get_max_threads();
    omp_set_num_threads(OPT::nMaxThreads);

    // Set arch type (2-compressed binary)
    OPT::nArchiveType = 2;

    OPT::nFusionMode = 0;
    OPT::thFilterPointCloud = 0; // TODO : consider adding this as user param

    return true;
}

bool PointCloudDensify::DensifyPointCloud(QString _scene_dir, QString _scene_file)
{

emit signal_userInformation("PointCloudDensify - Densify");
emit signal_processCompletion(-1);

QFileInfo scene_file_info(_scene_dir + QDir::separator() + _scene_file);

QString fullpath_basename = scene_file_info.absoluteDir().absoluteFilePath(scene_file_info.baseName());

// backup current path & set new one
namespace fs = boost::filesystem;
fs::path cur_working_dir(fs::current_path());
fs::current_path(fs::path(_scene_dir.toStdString() ));

Scene scene(OPT::nMaxThreads);

// load and estimate a dense point-cloud
if (!scene.Load(scene_file_info.absoluteFilePath().toStdString()))
{
    fs::current_path(cur_working_dir); // restore path
    return false;
}

if (scene.pointcloud.IsEmpty()) {
    fs::current_path(cur_working_dir); // restore path
    return false;
}
if (OPT::thFilterPointCloud < 0) {
    // filter point-cloud based on camera-point visibility intersections
    scene.PointCloudFilter(OPT::thFilterPointCloud);
    //const String baseFileName(MAKE_PATH_SAFE(Util::getFileFullName(OPT::strOutputFileName)) + _T("_filtered"));
 
    scene.Save((fullpath_basename + QString("_dense.mvs")).toStdString(), (ARCHIVE_TYPE)OPT::nArchiveType);
    scene.pointcloud.Save((fullpath_basename + QString("_dense.ply")).toStdString());

    fs::current_path(cur_working_dir); // restore path
    return true;
}
if ((ARCHIVE_TYPE) OPT::nArchiveType != ARCHIVE_MVS) {

    if (!scene.DenseReconstruction(OPT::nFusionMode)) {
        if (ABS(OPT::nFusionMode) != 1)
        {
            fs::current_path(cur_working_dir); // restore path
            return false;
        }

        fs::current_path(cur_working_dir); // restore path
        return true;
    }
}

// save the final mesh
scene.Save((fullpath_basename + QString("_dense.mvs")).toStdString(), (ARCHIVE_TYPE)OPT::nArchiveType);
scene.pointcloud.Save((fullpath_basename + QString("_dense.ply")).toStdString());

#if 0
scene.ExportCamerasMLP(baseFileName + _T(".mlp"), baseFileName + _T(".ply"));
#endif

    fs::current_path(cur_working_dir); // restore path
    return false;
}

bool PointCloudDensify::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // get params
    m_source_dir = absoluteDatasetDir();

    m_outdir = absoluteOutputTempDir();

    m_out_filename_prefix = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    return true;
}

bool PointCloudDensify::stop()
{
    return true;
}

void PointCloudDensify::onFlush(quint32 port)
{
    Q_UNUSED(port)

    // Log
    QString proc_info = logPrefix() + "Point cloud densification started\n";
    emit signal_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
    {
        fatalErrorExit("Reconstruction context not found !");
        return;
    }

    // init densify
    this->initDensify();

    // loop on all connected components
    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        QString scene_dir_i = m_outdir + QDir::separator() + "ModelPart" + QString("_%1").arg(rc->components_ids[i]);

        if (rc->current_format == ReconFormat::openMVG)
        {

            QString undist_out_dir_i = scene_dir_i + SEP + "undist_imgs";
            QString sfm_data_file = scene_dir_i + SEP + "sfm_data.bin";
            QString mvs_data_file = scene_dir_i + SEP + m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + rc->out_file_suffix + ".mvs";

            // Read the input SfM scene
            SfM_Data sfm_data;
            if (!Load(sfm_data, sfm_data_file.toStdString(), ESfM_Data(ALL))) {
                continue;
            }

            // Convert from openMVG to openMVS
            if (!exportToOpenMVS(sfm_data,
                mvs_data_file.toStdString(),
                undist_out_dir_i.toStdString(),
                0,
                this
            ))
                continue;


        }
        else
        {
            fatalErrorExit("Input point Cloud is not in the right format. Only openMVG supported for now");
        }

        emit signal_processCompletion(-1);
        emit signal_userInformation("PointCloudDensify");

        // compute dense scene
        if (!this->DensifyPointCloud(scene_dir_i, m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + rc->out_file_suffix + ".mvs") )
            continue;

  

    }

    rc->current_format = ReconFormat::openMVS;
    rc->out_file_suffix = rc->out_file_suffix + "_dense";

    emit signal_userInformation("PointCloudDensify");
    emit signal_processCompletion(100);

    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit signal_addToLog(proc_info);

    // Flush next module port
    flush(0);

}

} // namespace matisse

