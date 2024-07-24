#include "texture_mesh.h"
#include "reconstruction_context.h"
#include "nav_image.h"



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

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(TextureMesh, TextureMesh)
#endif

namespace OPT {
    String strInputFileName;
    String strMeshFileName;
    String strOutputFileName;
    String strViewsFileName;
    float fDecimateMesh;
    unsigned nCloseHoles;
    unsigned nResolutionLevel;
    unsigned nMinResolution;
    unsigned minCommonCameras;
    float fOutlierThreshold;
    float fRatioDataSmoothness;
    bool bGlobalSeamLeveling;
    bool bLocalSeamLeveling;
    unsigned nTextureSizeMultiple;
    unsigned nRectPackingHeuristic;
    uint32_t nColEmpty;
    float fSharpnessWeight;
    int nIgnoreMaskLabel;
    unsigned nOrthoMapResolution;
    unsigned nArchiveType;
    int nProcessPriority;
    unsigned nMaxThreads;
    int nMaxTextureSize;
    String strExportType;
    String strConfigFileName;
    boost::program_options::variables_map vm;
} // namespace OPT

namespace matisse {

TextureMesh::TextureMesh() :
    Processor(NULL, "TextureMesh", "Create a textured mesh from mesh and images", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");
	
}

TextureMesh::~TextureMesh(){

}

bool TextureMesh::configure()
{
    return true;
}

void TextureMesh::onNewImage(quint32 _port, matisse_image::Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);

}

bool TextureMesh::initTexturing()
{
	// initialize log and console
	OPEN_LOG();
	//OPEN_LOGCONSOLE();

	
	OPT::strExportType = "obj"; // Todo put this as an export param
	OPT::nArchiveType = ARCHIVE_MVS;
	OPT::nProcessPriority = -1;
	OPT::nMaxThreads = 0;
	OPT::fDecimateMesh = 1.f; // no decimation
	OPT::nCloseHoles = 30;
	OPT::nResolutionLevel = 0; // no rescale
	OPT::nMinResolution = 640;
	OPT::fOutlierThreshold = 6e-2f;
	OPT::fRatioDataSmoothness = 0.1f;
	OPT::minCommonCameras = 0; // texture even if no overlap with other view
	OPT::bGlobalSeamLeveling = true;
	OPT::bLocalSeamLeveling = true;
	OPT::nTextureSizeMultiple = 0; // texture size should be a multiple of this value (0 - power of two)
	OPT::nRectPackingHeuristic = 3;
	OPT::nColEmpty = 0x00FF7F27; // orange
	OPT::fSharpnessWeight = 0.5f;
	OPT::nOrthoMapResolution = 0;
	OPT::nIgnoreMaskLabel = -1;
	OPT::nMaxTextureSize = 8192;

	INIT_WORKING_FOLDER;

	// initialize the log file
	OPEN_LOGFILE(MAKE_PATH("TextureMesh" _T("-") + Util::getUniqueName(0) + _T(".log")).c_str());

	// print application details: version and command line
	Util::LogBuild();

    //OPT::strInputFileName = ".....";
	OPT::strMeshFileName = Util::getFileFullName(OPT::strInputFileName) + _T(".ply");
	OPT::strOutputFileName = Util::getFileFullName(OPT::strInputFileName) + _T("_texture.mvs");

	MVS::Initialize("TextureMesh", OPT::nMaxThreads, OPT::nProcessPriority);
	return true;
}

bool TextureMesh::textureMesh(QString& _mvs_data_file, QString& _mesh_filepath, QString& _output_textured_file)
{
	QFileInfo mvs_file_info(_mvs_data_file);

	Scene scene(OPT::nMaxThreads);
	// load and texture the mesh
	const Scene::SCENE_TYPE sceneType(scene.Load(_mvs_data_file.toStdString()));
	if (sceneType == Scene::SCENE_NA)
		return false;
	if (!OPT::strMeshFileName.empty() && !scene.mesh.Load(_mesh_filepath.toStdString())) {
		return false;
	}
	if (scene.mesh.IsEmpty()) {
		return false;
	}
	//const String baseFileName(MAKE_PATH_SAFE(Util::getFileFullName(OPT::strOutputFileName)));

	{
		// decimate to the desired resolution
		if (OPT::fDecimateMesh < 1.f) {
			ASSERT(OPT::fDecimateMesh > 0.f);
			scene.mesh.Clean(OPT::fDecimateMesh, 0.f, false, OPT::nCloseHoles, 0u, 0.f, false);
			scene.mesh.Clean(1.f, 0.f, false, 0u, 0u, 0.f, true); // extra cleaning to remove non-manifold problems created by closing holes
		}

		IIndexArr views;
		// compute mesh texture
		TD_TIMER_START();
		if (!scene.TextureMesh(OPT::nResolutionLevel, OPT::nMinResolution, OPT::minCommonCameras, OPT::fOutlierThreshold, OPT::fRatioDataSmoothness,
			OPT::bGlobalSeamLeveling, OPT::bLocalSeamLeveling, OPT::nTextureSizeMultiple, OPT::nRectPackingHeuristic, Pixel8U(OPT::nColEmpty),
			OPT::fSharpnessWeight, OPT::nIgnoreMaskLabel, OPT::nMaxTextureSize, views))
			return false;
		VERBOSE("Mesh texturing completed: %u vertices, %u faces (%s)", scene.mesh.vertices.GetSize(), scene.mesh.faces.GetSize(), TD_TIMER_GET_FMT().c_str());

		// save the final mesh
		scene.mesh.Save(_output_textured_file.toStdString());

        //if ((ARCHIVE_TYPE)OPT::nArchiveType != ARCHIVE_MVS || sceneType != Scene::SCENE_INTERFACE)
        //    scene.Save(baseFileName + _T(".mvs"), (ARCHIVE_TYPE)OPT::nArchiveType);
    }


    // Commented for now but would be interesting to activate on option for the future
	//if (OPT::nOrthoMapResolution) {
	//	// project mesh as an orthographic image
	//ProjectOrtho:
	//	Image8U3 imageRGB;
	//	Image8U imageRGBA[4];
	//	Point3 center;
	//	scene.mesh.ProjectOrthoTopDown(OPT::nOrthoMapResolution, imageRGB, imageRGBA[3], center);
	//	Image8U4 image;
	//	cv::split(imageRGB, imageRGBA);
	//	cv::merge(imageRGBA, 4, image);
	//	image.Save(baseFileName + _T("_orthomap.png"));
	//	SML sml(_T("OrthoMap"));
	//	sml[_T("Center")].val = String::FormatString(_T("%g %g %g"), center.x, center.y, center.z);
	//	sml.Save(baseFileName + _T("_orthomap.txt"));
	//}

	return true;
}

bool TextureMesh::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // get params
    m_source_dir = absoluteDatasetDir();

    m_outdir = absoluteOutputTempDir();

    m_out_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    return true;
}

bool TextureMesh::stop()
{
    return true;
}

void TextureMesh::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    QElapsedTimer timer;
    timer.start();

    static const QString qsep = QDir::separator();

    emit si_processCompletion(0);
    emit si_userInformation("TextureMesh - start");

    QString proc_info = logPrefix() + "TextureMesh started\n";
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


    // loop on all connected components
    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        QString scene_dir_i = m_outdir + QDir::separator() + QString("openmvs_result_%1").arg(rc->components_ids[i]);
        //QString mvs_data_file = scene_dir_i + qsep + QString("scene")+ rc->out_file_suffix + ".mvs";
        //QString mesh_data_file = scene_dir_i + qsep + QString("scene") + rc->out_file_suffix + ".ply";
        //QString textured_data_file = scene_dir_i + qsep + QString("scene") + rc->out_file_suffix + "_textured.obj";

        QString mvs_data_file = QString("scene") + rc->out_file_suffix + ".mvs";
        QString mesh_data_file = QString("scene") + rc->out_file_suffix + ".ply";
        QString textured_data_file = QString("%1_%2%3_textured.obj").arg(m_out_filename_prefix).arg(rc->components_ids[i]).arg(rc->out_file_suffix);

        WORKING_FOLDER = scene_dir_i.toStdString();
        this->initTexturing();

        if (rc->current_format != ReconFormat::openMVS)
        {
            fatalErrorExit("Input point Cloud is not in the right format. Only openMVS supported for now");
            return;
        }

        // backup current path & set new one (so that openMVS find images)
        namespace fs = boost::filesystem;
        fs::path cur_working_dir(fs::current_path());
        fs::current_path(fs::path(scene_dir_i.toStdString()));

        // compute dense scene
        if (!this->textureMesh(mvs_data_file, mesh_data_file, textured_data_file) )
            continue;

        // Finalize
        MVS::Finalize();

        CLOSE_LOGFILE();
        //CLOSE_LOGCONSOLE();
        CLOSE_LOG();

        // restore path
        fs::current_path(cur_working_dir);

        //// Compute Mesh
        emit si_userInformation("Meshing...");
        emit si_processCompletion(-1);

        emit si_processCompletion(100);

    }

    // complete current file suffix
    rc->out_file_suffix += "_textured";

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);
    //qDebug() << logPrefix() << " took " << timer.elapsed() / 1000.0 << " seconds";

    // Flush next module port
//    flush(0);

}

} // namespace matisse

