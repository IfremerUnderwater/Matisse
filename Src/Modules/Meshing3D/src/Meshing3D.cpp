#include "Meshing3D.h"
#include "reconstructioncontext.h"
#include "NavImage.h"

#define OPENMVG_USE_OPENMP
#include "MVGMVSInterface.h"

#include <QProcess>

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
Q_EXPORT_PLUGIN2(Meshing3D, Meshing3D)
#endif

// traiter les étoiles (total 51 étoiles pour 100 %)
// 0%   10   20   30   40   50   60   70   80   90   100%
// |----|----|----|----|----|----|----|----|----|----|
// ***************************************************
// entrée : message
// mis à jour : starcount
// retourne : le pourcentage d'avancement
static int progressStarCountPct(QString message, int &starcount)
{
    int n = message.count("*");
    starcount += n;
    double pct = starcount / 51.0 * 100.0;
    if(starcount > 51)
        starcount %= 52;
    return int(pct);
}

// type
// "Estimated depth-maps 1 (6.25%, 499ms, ETA 7s)..."
//
static double getPctVal(QString message, QString key)
{
    int pos = message.indexOf(key);
    QString substr = message.mid(pos);
    pos = substr.indexOf("\n");
    if(pos != -1)
        substr = substr.left(pos);
    pos = substr.indexOf("(");
    if(pos != -1)
        substr = substr.mid(pos+1);
    pos = substr.indexOf("%");
    if(pos != -1)
        substr = substr.left(pos);
    bool ok;
    double val = substr.toDouble(&ok);

    if(ok)
        return val;
    else
        return -1;
}

#ifdef _WIN32
static const char* DensifyPointCloudExe = "DensifyPointCloud.exe";
static const char* ReconstructMeshExe = "ReconstructMesh.exe";
#else
static const char* DensifyPointCloudExe = "DensifyPointCloud";
static const char* ReconstructMeshExe = "ReconstructMesh";
#endif

Meshing3D::Meshing3D() :
    Processor(NULL, "Meshing3D", "Create a mesh from 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "resolution_level");
    addExpectedParameter("algo_param", "number_views_fuse");
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

bool Meshing3D::initDensify()
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

bool Meshing3D::densifyPointCloud(QString _scene_dir, QString _scene_file)
{

emit signal_userInformation("Meshing3D - Densify");
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

    // init densify
    this->initDensify();

    // loop on all connected components
    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        //emit signal_userInformation("Meshing3D - convert");
        //emit signal_processCompletion(0);

        QString scene_dir_i = m_outdir + QString("_%1").arg(rc->components_ids[i]);
        QString undist_out_dir_i = scene_dir_i + SEP + "undist_imgs_for_mvs";
        QString sfm_data_file = scene_dir_i + SEP + "sfm_data.bin";
        QString mvs_data_file = scene_dir_i + SEP + m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + ".mvs";

        // Read the input SfM scene
        SfM_Data sfm_data;
        if (!Load(sfm_data, sfm_data_file.toStdString(), ESfM_Data(ALL))) {
            continue;
        }

        // Convert from openMVG to openMVS
        if (!exportToOpenMVS(                    sfm_data,
                              mvs_data_file.toStdString(),
                           undist_out_dir_i.toStdString(),
                                                        0,
                                                      this
        ))
            continue;

        // compute dense scene
        if (!this->densifyPointCloud(scene_dir_i, m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + ".mvs") )
            continue;

        //// Compute Mesh
        //emit signal_userInformation("Meshing3D - Compute Mesh");
        //emit signal_processCompletion(0);

        //cmdline = ReconstructMeshExe;
        //double decimatearg = _matisseParameters->getDoubleParamValue("algo_param", "decimate_factor", ok);
        //if(ok)
        //    cmdline += " --decimate " + QString::number(decimatearg);

        //cmdline +=  " ."+SEP+ m_outdir+QString("_%1").arg(rc->components_ids[i]) +SEP+ m_out_filename_prefix+QString("_%1").arg(rc->components_ids[i]) + "_dense.mvs";
        //QProcess meshProc;
        //meshProc.setWorkingDirectory(m_source_dir);
        //meshProc.start(cmdline);

        //while(meshProc.waitForReadyRead(-1)){

        //    if(!isStarted())
        //    {
        //        meshProc.kill();
        //        fatalErrorExit("ReconstructMesh Cancelled");
        //        return;
        //    }

        //    QString output = meshProc.readAllStandardOutput();

        //    // étape "Points inserted "
        //    if(output.contains("Points inserted "))
        //    {
        //        double val = getPctVal(output,"Points inserted ");
        //        if(val >= 0)
        //        {
        //            emit signal_userInformation("Rec. Mesh - Points inserted");
        //            emit signal_processCompletion((int)val);
        //        }
        //    }

        //    // étape "Points weighted "
        //    if(output.contains("Points weighted "))
        //    {
        //        double val = getPctVal(output,"Points weighted ");
        //        if(val >= 0)
        //        {
        //            emit signal_userInformation("Rec. Mesh - Points weighted");
        //            emit signal_processCompletion((int)val);
        //        }
        //    }

        //    if(output.contains("Delaunay tetrahedras weighting completed:"))
        //    {
        //        emit signal_processCompletion((quint8)-1);
        //        emit signal_userInformation("Rec. Mesh - tetra. weighting");
        //    }

        //    if(output.contains("Delaunay tetrahedras graph-cut completed"))
        //    {
        //        emit signal_processCompletion((quint8)-1);
        //        emit signal_userInformation("Rec. Mesh - tetra. graph-cut");
        //    }

        //    if(output.contains("Mesh reconstruction completed:"))
        //    {
        //        emit signal_processCompletion((quint8)-1);
        //        emit signal_userInformation("Rec. Mesh - reconstr. complete");
        //    }

        //    qDebug() << output;
        //}

        emit signal_userInformation("Meshing3D - end");
        emit signal_processCompletion(100);

    }

    // Flush next module port
    flush(0);

}

