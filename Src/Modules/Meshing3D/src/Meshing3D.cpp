#include "Meshing3D.h"
#include "reconstructioncontext.h"
#include "NavImage.h"

#define OPENMVG_USE_OPENMP
#include "MVGMVSInterface.h"

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
    float fDistInsert;
    bool bUseConstantWeight;
    bool bUseFreeSpaceSupport;
    float fThicknessFactor;
    float fQualityFactor;
    float fDecimateMesh;
    float fRemoveSpurious;
    bool bRemoveSpikes;
    unsigned nCloseHoles;
    unsigned nSmoothMesh;
    unsigned nArchiveType;
    int nProcessPriority;
    unsigned nMaxThreads;
    String strExportType;
    String strConfigFileName;
} // namespace OPT


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

void Meshing3D::onNewImage(quint32 port, MatisseCommon::Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);

}

bool Meshing3D::initMeshing()
{
    OPT::fDistInsert=2.5f;
    OPT::bUseConstantWeight=true;
    OPT::bUseFreeSpaceSupport=false;
    OPT::fThicknessFactor=1.f;
    OPT::fQualityFactor=1.f;
    OPT::fDecimateMesh=1.f;
    OPT::fRemoveSpurious=20.f;
    OPT::bRemoveSpikes=true;
    OPT::nCloseHoles=30;
    OPT::nSmoothMesh=2;
    OPT::nArchiveType=2;
    OPT::nMaxThreads=0;
    OPT::strExportType="ply";

    bool ok;
    double decimatearg = _matisseParameters->getDoubleParamValue("algo_param", "decimate_factor", ok);
    if (ok)
        OPT::fDecimateMesh = decimatearg;

    // Set max threads
    OPT::nMaxThreads = omp_get_max_threads();
    omp_set_num_threads(OPT::nMaxThreads);

    return true;
}

bool Meshing3D::meshing(QString _mvs_data_file)
{
	QFileInfo mvs_file_info(_mvs_data_file);

	MVS::Scene scene(OPT::nMaxThreads);
	// load project
	if (!scene.Load(_mvs_data_file.toStdString()))
		return false;

	// reset image resolution to the original size and
	// make sure the image neighbors are initialized before deleting the point-cloud
	bool bAbort(false);
#pragma omp parallel for
	for (int_t idx = 0; idx < (int_t)scene.images.GetSize(); ++idx) {
#pragma omp flush (bAbort)
		if (bAbort)
			continue;
		const uint32_t idxImage((uint32_t)idx);

		MVS::Image& imageData = scene.images[idxImage];
		if (!imageData.IsValid())
			continue;
		// reset image resolution
		if (!imageData.ReloadImage(0, false)) {
			bAbort = true;
#pragma omp flush (bAbort)
			continue;
		}
		imageData.UpdateCamera(scene.platforms);
		// select neighbor views
		if (imageData.neighbors.IsEmpty()) {
			IndexArr points;
			scene.SelectNeighborViews(idxImage, points);
		}
	}

	if (bAbort)
		return false;
	// reconstruct a coarse mesh from the given point-cloud
	if (OPT::bUseConstantWeight)
		scene.pointcloud.pointWeights.Release();
	if (!scene.ReconstructMesh(OPT::fDistInsert, OPT::bUseFreeSpaceSupport, 4, OPT::fThicknessFactor, OPT::fQualityFactor))
		return false;

	// clean the mesh
	scene.mesh.Clean(OPT::fDecimateMesh, OPT::fRemoveSpurious, OPT::bRemoveSpikes, OPT::nCloseHoles, OPT::nSmoothMesh, false);
	scene.mesh.Clean(1.f, 0.f, OPT::bRemoveSpikes, OPT::nCloseHoles, 0, false); // extra cleaning trying to close more holes
	scene.mesh.Clean(1.f, 0.f, false, 0, 0, true); // extra cleaning to remove non-manifold problems created by closing holes

	// save the final mesh
    QString output_filename = mvs_file_info.dir().absoluteFilePath(mvs_file_info.baseName() + "_mesh");
	scene.Save(output_filename.toStdString()+".mvs", (ARCHIVE_TYPE)OPT::nArchiveType);
	scene.mesh.Save(output_filename.toStdString() +"." + OPT::strExportType);


	return true;
}

bool Meshing3D::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // get params
    m_source_dir = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    m_outdir = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if (m_outdir.isEmpty())
        m_outdir = "outReconstruction";

    m_outdir = m_source_dir + SEP + m_outdir;

    m_out_filename_prefix = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    return true;
}

bool Meshing3D::stop()
{
    return true;
}

void Meshing3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();

    emit signal_processCompletion(0);
    emit signal_userInformation("Meshing3D - start");

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

    this->initMeshing();

    // loop on all connected components
    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        QString scene_dir_i = m_outdir + QString("_%1").arg(rc->components_ids[i]);
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
        emit signal_userInformation("Meshing...");
        emit signal_processCompletion(-1);

        emit signal_processCompletion(100);

    }

    // complete current file suffix
    rc->out_file_suffix += "_mesh";

    qDebug() << logPrefix() << " took " << timer.elapsed() / 1000.0 << " seconds";

    // Flush next module port
    flush(0);

}

