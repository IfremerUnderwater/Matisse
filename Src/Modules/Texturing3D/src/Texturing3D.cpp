#include "Texturing3D.h"
#include "reconstructioncontext.h"
#include "NavImage.h"

#include <QFile>
#include <QProcess>
#include <QElapsedTimer>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include "openMVG/cameras/Camera_Pinhole.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include <fstream>


#ifdef _WIN32
static const char* TexreconExe = "texrecon.exe";
#define MVE_EMBEDDED "MVE==undistorted "
#else
static const char* TexreconExe = "texrecon";
#define MVE_EMBEDDED "MVE::undistorted "
#endif

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::geometry;
using namespace openMVG::sfm;


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Texturing3D, Texturing3D)
#endif

// proccess stars (total 51 stars for 100 %)
// 0%   10   20   30   40   50   60   70   80   90   100%
// |----|----|----|----|----|----|----|----|----|----|
// ***************************************************
static int progressStarCountPct(QString message, int &starcount)
{
    int n = message.count("*");
    starcount += n;
    double pct = starcount / 51.0 * 100.0;
    if(starcount > 51)
        starcount %= 52;
    return int(pct);
}

Texturing3D::Texturing3D() :
    Processor(NULL, "Texturing3D", "T 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "keep_unseen_faces");
}

Texturing3D::~Texturing3D(){

}

bool Texturing3D::configure()
{
    return true;
}

void Texturing3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool Texturing3D::generateCamFile(QString _sfm_data_file, QString _undist_img_path)
{

    // Create output dir
    if (!stlplus::folder_exists(_undist_img_path.toStdString()))
        return false; // should have been created for mvs proc

    // Read the SfM scene
    SfM_Data sfm_data;
    if (!Load(sfm_data, _sfm_data_file.toStdString(), ESfM_Data(VIEWS | INTRINSICS | EXTRINSICS))) {
        std::cerr << std::endl
            << "The input SfM_Data file \"" << _sfm_data_file.toStdString() << "\" cannot be read." << std::endl;
        return false;
    }

    for (Views::const_iterator iter = sfm_data.GetViews().begin();
        iter != sfm_data.GetViews().end(); ++iter)
    {
        const View* view = iter->second.get();
        if (!sfm_data.IsPoseAndIntrinsicDefined(view))
            continue;

        // Valid view, we can ask a pose & intrinsic data
        const Pose3 pose = sfm_data.GetPoseOrDie(view);
        Intrinsics::const_iterator iterIntrinsic = sfm_data.GetIntrinsics().find(view->id_intrinsic);
        const IntrinsicBase* cam = iterIntrinsic->second.get();

        if (!cameras::isPinhole(cam->getType()))
            continue;
        const Pinhole_Intrinsic* pinhole_cam = static_cast<const Pinhole_Intrinsic*>(cam);

        // Extrinsic
        const Vec3 t = pose.translation();
        const Mat3 R = pose.rotation();
        // Intrinsic
        const double f = pinhole_cam->focal();
        const Vec2 pp = pinhole_cam->principal_point();

        // Image size in px
        const int w = pinhole_cam->w();
        const int h = pinhole_cam->h();

        // We can now create the .cam file for the View in the output dir
        std::ofstream outfile(stlplus::create_filespec(
            _undist_img_path.toStdString(), stlplus::basename_part(view->s_Img_path), "cam").c_str());
        // See https://github.com/nmoehrle/mvs-texturing/blob/master/apps/texrecon/arguments.cpp
        // for full specs
        const int largerDim = w > h ? w : h;
        outfile << t(0) << " " << t(1) << " " << t(2) << " "
            << R(0, 0) << " " << R(0, 1) << " " << R(0, 2) << " "
            << R(1, 0) << " " << R(1, 1) << " " << R(1, 2) << " "
            << R(2, 0) << " " << R(2, 1) << " " << R(2, 2) << "\n"
            << f / largerDim << " 0 0 1 " << pp(0) / w << " " << pp(1) / h;
        outfile.close();

    }

    return true;
}

void Texturing3D::writeKml(QString model_path, QString model_prefix)
{
    // Write kml for model
    QVariant *object = _context->getObject("reconstruction_context");
    if (object) {
        reconstructionContext * rc = object->value<reconstructionContext*>();
        QFile kml_file(model_path+QDir::separator()+model_prefix+QString(".kml"));
        if( !kml_file.open(QIODevice::WriteOnly) )
        {
            fatalErrorExit("Error saving " + model_prefix);
        }

        // Save kml info
        QTextStream outputGeoStream(&kml_file);

        outputGeoStream << QString("<kml>\n");
        outputGeoStream << QString("  <!--Generated with MATISSE 3D -->\n");
        outputGeoStream << QString("  <Placemark>\n");
        outputGeoStream << QString("    <name>%1</name>\n").arg(model_prefix);
        outputGeoStream << QString("    <Model>\n");
        outputGeoStream << QString("      <altitudeMode>absolute</altitudeMode>\n");
        outputGeoStream << QString("      <Location>\n");
        outputGeoStream << QString("        <longitude>%1</longitude>\n").arg(QString::number(rc->lon_origin, 'f', 8));
        outputGeoStream << QString("        <latitude>%1</latitude>\n").arg(QString::number(rc->lat_origin, 'f', 8));
        outputGeoStream << QString("        <altitude>%1</altitude>\n").arg(QString::number(rc->alt_origin, 'f', 3));
        outputGeoStream << QString("      </Location>\n");
        outputGeoStream << QString("      <Orientation>\n");
        outputGeoStream << QString("        <heading>0</heading>\n");
        outputGeoStream << QString("        <tilt>0</tilt>\n");
        outputGeoStream << QString("        <roll>0</roll>\n");
        outputGeoStream << QString("      </Orientation>\n");
        outputGeoStream << QString("      <Scale>\n");
        outputGeoStream << QString("        <x>1</x>\n");
        outputGeoStream << QString("        <y>1</y>\n");
        outputGeoStream << QString("        <z>1</z>\n");
        outputGeoStream << QString("      </Scale>\n");
        outputGeoStream << QString("      <Link>\n");
        outputGeoStream << QString("        <href>%1.obj</href>\n").arg(model_prefix);
        outputGeoStream << QString("      </Link>\n");
        outputGeoStream << QString("    </Model>\n");
        outputGeoStream << QString("  </Placemark>\n");
        outputGeoStream << QString("</kml>\n");

        kml_file.close();
    }
}

bool Texturing3D::start()
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

bool Texturing3D::stop()
{
    return true;
}

void Texturing3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    // Log
    QString proc_info = logPrefix() + "Texturing started\n";
    emit signal_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();

    emit signal_processCompletion(0);
    emit signal_userInformation("Texturing3D start");

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
        fatalErrorExit("Context issue ...");

    for(unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        // Convert model to mvs-texturing
        emit signal_userInformation("Texturing3D convert");
        
        QString scene_dir_i = m_outdir + QString("_%1").arg(rc->components_ids[i]);
        QString mesh_data_file = scene_dir_i + SEP + m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + rc->out_file_suffix + ".ply";
        QString undist_dir_i = scene_dir_i + SEP + "undist_imgs";
        QString sfm_data_file = scene_dir_i + SEP + "sfm_data.bin";

        if (!this->generateCamFile(sfm_data_file, undist_dir_i))
            continue;

        // Texture model
        emit signal_userInformation("Texturing3D texturing");

        QString cmdLine = TexreconExe;
        bool ok = false;
        bool keep_unseen_faces = _matisseParameters->getBoolParamValue("algo_param", "keep_unseen_faces", ok);
        if(ok && keep_unseen_faces)
            cmdLine += " --keep_unseen_faces";
        cmdLine += " " + undist_dir_i;
        cmdLine += " "+ mesh_data_file;
        cmdLine += " "+ m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + "_texrecon";
        QProcess textureProc;
        textureProc.setWorkingDirectory(scene_dir_i);
        textureProc.start(cmdLine);

        bool initer = false;
        while(textureProc.waitForReadyRead(-1)){
            if(!isStarted())
            {
                textureProc.kill();
                fatalErrorExit("texrecon Cancelled");
                return;
            }

            QString output = textureProc.readAllStandardOutput();

            if(output.contains("Load and prepare mesh:"))
            {
                // init
                emit signal_processCompletion((qint8)-1);
                emit signal_userInformation("texrecon : Load mesh...");
            }

            if(output.contains("Generating texture views:"))
            {
                emit signal_userInformation("texrecon : Gen. texture views...");
            }

            if(output.contains("Building adjacency graph:"))
            {
                emit signal_userInformation("texrecon : Build. adj graphs...");
            }

            if(output.contains("View selection:"))
            {
                emit signal_userInformation("texrecon : View selection...");
            }

            if(output.contains("\tCalculating face qualities "))
            {
                emit signal_userInformation("texrecon : calc face qual...");
            }

            if(output.contains("\tWriting data cost file"))
            {
                initer = true;
                emit signal_userInformation("texrecon : writing data cost...");
            }

            if(initer && output.startsWith("\t"))
            {
                // format
                // "    Comp	Iter	Energy		Runtime"
                // "	0000	0	1.10237e+06	0.017"
                QStringList list = output.mid(1).split("\t");
                if(list.count() > 2)
                    emit signal_userInformation("texrecon : view sel. iter " + list.at(1));
            }

            if(output.contains("Generating texture patches:"))
            {
                initer = false;
                emit signal_userInformation("texrecon : gen text. patches...");
            }

            if(output.contains("Running global seam leveling:"))
            {
                initer = false;
                emit signal_userInformation("texrecon : global seam level...");
            }

            if(output.contains("\tAdjusting texture patches"))
            {
                initer = false;
                emit signal_userInformation("texrecon : adj text. patches...");
            }

            if(output.contains("Running local seam leveling:"))
            {
                initer = false;
                emit signal_userInformation("texrecon : local seam level...");
            }

            if(output.contains("Generating texture atlases:"))
            {
                initer = false;
                emit signal_userInformation("texrecon : gen. text. atlas...");
            }

            if(output.contains("\tFinalizing texture atlases"))
            {
                initer = false;
                emit signal_userInformation("texrecon : final. text. atlas...");
            }

            if(output.contains("Building objmodel:"))
            {
                initer = false;
                emit signal_userInformation("texrecon : build objmodel...");
            }

            qDebug() << output;
        }

        // Write kml associated to model
        writeKml(scene_dir_i, m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + "_texrecon");

        emit signal_processCompletion(100);
        emit signal_userInformation("Texturing3D end");

    }

    // update suffix
    rc->out_file_suffix = "_texrecon";

    qDebug() << logPrefix() << " took " << timer.elapsed() / 1000.0 << " seconds";

    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit signal_addToLog(proc_info);

    // Flush next module port
    flush(0);

}

