#include "meshing_3d.h"
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
#define _USE_OPENMP
#define RECMESH_USE_OPENMP
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "OpenMVS/MVS/Common.h"
#include "OpenMVS/MVS/Scene.h"
#include "omp.h"
#include <QFileInfo>

using namespace MVS;

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::geometry;
using namespace openMVG::image;
using namespace openMVG::sfm;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Meshing3D, Meshing3D)
#endif

namespace OPT {
    float f_dist_insert;
    bool b_use_constant_weight;
    bool b_use_free_space_support;
    float f_thickness_factor;
    float f_quality_factor;
    float f_decimate_mesh;
    float f_remove_spurious;
    bool b_remove_spikes;
    unsigned n_close_holes;
    unsigned n_smooth_mesh;
    unsigned n_archive_type;
    int n_process_priority;
    unsigned n_max_threads;
    String str_export_type;
    String str_config_file_name;
} // namespace OPT

namespace matisse {

Meshing3D::Meshing3D() :
    Processor(NULL, "Meshing3D", "Create a mesh from 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "decimate_factor");
}

Meshing3D::~Meshing3D(){

}

bool Meshing3D::configure()
{
    return true;
}

void Meshing3D::onNewImage(quint32 _port, matisse_image::Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);

}

bool Meshing3D::initMeshing()
{
    OPT::f_dist_insert=2.5f;
    OPT::b_use_constant_weight=true;
    OPT::b_use_free_space_support=false;
    OPT::f_thickness_factor=1.f;
    OPT::f_quality_factor=1.f;
    OPT::f_decimate_mesh=1.f;
    OPT::f_remove_spurious=20.f;
    OPT::b_remove_spikes=true;
    OPT::n_close_holes=30;
    OPT::n_smooth_mesh=2;
    OPT::n_archive_type=2;
    OPT::n_max_threads=0;
    OPT::str_export_type="ply";

    bool ok;
    double decimatearg = m_matisse_parameters->getDoubleParamValue("algo_param", "decimate_factor", ok);
    if (ok)
        OPT::f_decimate_mesh = decimatearg;

    // Set max threads
    OPT::n_max_threads = omp_get_max_threads();
    omp_set_num_threads(OPT::n_max_threads);

    return true;
}

bool Meshing3D::meshing(QString _mvs_data_file)
{
	QFileInfo mvs_file_info(_mvs_data_file);

    MVS::Scene scene(OPT::n_max_threads);
	// load project
	if (!scene.Load(_mvs_data_file.toStdString()))
		return false;

	// reset image resolution to the original size and
	// make sure the image neighbors are initialized before deleting the point-cloud
    bool b_abort(false);
#pragma omp parallel for
	for (int_t idx = 0; idx < (int_t)scene.images.GetSize(); ++idx) {
#pragma omp flush (b_abort)
        if (b_abort)
			continue;
        const uint32_t idx_image((uint32_t)idx);

        MVS::Image& image_data = scene.images[idx_image];
        if (!image_data.IsValid())
			continue;
		// reset image resolution
        if (!image_data.ReloadImage(0, false)) {
            b_abort = true;
#pragma omp flush (b_abort)
			continue;
		}
        image_data.UpdateCamera(scene.platforms);
		// select neighbor views
        if (image_data.neighbors.IsEmpty()) {
			IndexArr points;
            scene.SelectNeighborViews(idx_image, points);
		}
	}

    if (b_abort)
		return false;
	// reconstruct a coarse mesh from the given point-cloud
    if (OPT::b_use_constant_weight)
		scene.pointcloud.pointWeights.Release();
    if (!scene.ReconstructMesh(OPT::f_dist_insert, OPT::b_use_free_space_support, 4, OPT::f_thickness_factor, OPT::f_quality_factor))
		return false;

	// clean the mesh
    scene.mesh.Clean(OPT::f_decimate_mesh, OPT::f_remove_spurious, OPT::b_remove_spikes, OPT::n_close_holes, OPT::n_smooth_mesh, false);
    scene.mesh.Clean(1.f, 0.f, OPT::b_remove_spikes, OPT::n_close_holes, 0, false); // extra cleaning trying to close more holes
	scene.mesh.Clean(1.f, 0.f, false, 0, 0, true); // extra cleaning to remove non-manifold problems created by closing holes

	// save the final mesh
    QString output_filename = mvs_file_info.dir().absoluteFilePath(mvs_file_info.baseName() + "_mesh");
    scene.Save(output_filename.toStdString()+".mvs", (ARCHIVE_TYPE)OPT::n_archive_type);
    scene.mesh.Save(output_filename.toStdString() +"." + OPT::str_export_type);


	return true;
}

bool Meshing3D::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // get params
    m_source_dir = absoluteDatasetDir();

    m_outdir = absoluteOutputTempDir();

    m_out_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    return true;
}

bool Meshing3D::stop()
{
    return true;
}

void Meshing3D::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();

    emit si_processCompletion(0);
    emit si_userInformation("Meshing3D - start");

    QString proc_info = logPrefix() + "Meshing started\n";
    emit si_addToLog(proc_info);

    // Get context
    QVariant *object = m_context->getObject("reconstruction_context");
    reconstructionContext * rc;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
    {
        fatalErrorExit("Reconstruction context not found !");
        return;
    }

    this->initMeshing();

    // loop on all connected components
    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        QString scene_dir_i = m_outdir +QDir::separator() + "ModelPart" + QString("_%1").arg(rc->components_ids[i]);
        QString mvs_data_file = scene_dir_i + SEP + m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + rc->out_file_suffix + ".mvs";

        if (rc->current_format == ReconFormat::openMVG)
        {

            QString undist_out_dir_i = scene_dir_i + SEP + "undist_imgs";
            QString sfm_data_file = scene_dir_i + SEP + "sfm_data.bin";

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
        else if (rc->current_format != ReconFormat::openMVS)
        {
            fatalErrorExit("Input point Cloud is not in the right format. Only openMVG and openMVS supported for now");
        }

        // compute dense scene
        if (!this->meshing(mvs_data_file) )
            continue;

        //// Compute Mesh
        emit si_userInformation("Meshing...");
        emit si_processCompletion(-1);

        emit si_processCompletion(100);

    }

    // complete current file suffix
    rc->out_file_suffix += "_mesh";

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);
    //qDebug() << logPrefix() << " took " << timer.elapsed() / 1000.0 << " seconds";

    // Flush next module port
//    flush(0);

}

} // namespace matisse

