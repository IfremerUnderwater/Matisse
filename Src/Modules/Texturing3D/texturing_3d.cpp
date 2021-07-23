#include "texturing_3d.h"
#include "reconstruction_context.h"
#include "nav_image.h"

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

namespace matisse {

// proccess stars (total 51 stars for 100 %)
// 0%   10   20   30   40   50   60   70   80   90   100%
// |----|----|----|----|----|----|----|----|----|----|
// ***************************************************
static int progressStarCountPct(QString _message, int &_starcount)
{
    int n = _message.count("*");
    _starcount += n;
    double pct = _starcount / 51.0 * 100.0;
    if(_starcount > 51)
        _starcount %= 52;
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

void Texturing3D::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
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
        Intrinsics::const_iterator iter_intrinsic = sfm_data.GetIntrinsics().find(view->id_intrinsic);
        const IntrinsicBase* cam = iter_intrinsic->second.get();

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
        const int larger_dim = w > h ? w : h;
        outfile << t(0) << " " << t(1) << " " << t(2) << " "
            << R(0, 0) << " " << R(0, 1) << " " << R(0, 2) << " "
            << R(1, 0) << " " << R(1, 1) << " " << R(1, 2) << " "
            << R(2, 0) << " " << R(2, 1) << " " << R(2, 2) << "\n"
            << f / larger_dim << " 0 0 1 " << pp(0) / w << " " << pp(1) / h;
        outfile.close();

    }

    return true;
}

void Texturing3D::writeKml(QString _model_path, QString _model_prefix)
{
    // Write kml for model
    QVariant *object = m_context->getObject("reconstruction_context");
    if (object) {
        reconstructionContext * rc = object->value<reconstructionContext*>();
        QFile kml_file(_model_path+QDir::separator()+_model_prefix+QString(".kml"));
        if( !kml_file.open(QIODevice::WriteOnly) )
        {
            fatalErrorExit("Error saving " + _model_prefix);
        }

        // Save kml info
        QTextStream output_geo_stream(&kml_file);

        output_geo_stream << QString("<kml>\n");
        output_geo_stream << QString("  <!--Generated with MATISSE 3D -->\n");
        output_geo_stream << QString("  <Placemark>\n");
        output_geo_stream << QString("    <name>%1</name>\n").arg(_model_prefix);
        output_geo_stream << QString("    <Model>\n");
        output_geo_stream << QString("      <altitudeMode>absolute</altitudeMode>\n");
        output_geo_stream << QString("      <Location>\n");
        output_geo_stream << QString("        <longitude>%1</longitude>\n").arg(QString::number(rc->lon_origin, 'f', 8));
        output_geo_stream << QString("        <latitude>%1</latitude>\n").arg(QString::number(rc->lat_origin, 'f', 8));
        output_geo_stream << QString("        <altitude>%1</altitude>\n").arg(QString::number(rc->alt_origin, 'f', 3));
        output_geo_stream << QString("      </Location>\n");
        output_geo_stream << QString("      <Orientation>\n");
        output_geo_stream << QString("        <heading>0</heading>\n");
        output_geo_stream << QString("        <tilt>0</tilt>\n");
        output_geo_stream << QString("        <roll>0</roll>\n");
        output_geo_stream << QString("      </Orientation>\n");
        output_geo_stream << QString("      <Scale>\n");
        output_geo_stream << QString("        <x>1</x>\n");
        output_geo_stream << QString("        <y>1</y>\n");
        output_geo_stream << QString("        <z>1</z>\n");
        output_geo_stream << QString("      </Scale>\n");
        output_geo_stream << QString("      <Link>\n");
        output_geo_stream << QString("        <href>%1.obj</href>\n").arg(_model_prefix);
        output_geo_stream << QString("      </Link>\n");
        output_geo_stream << QString("    </Model>\n");
        output_geo_stream << QString("  </Placemark>\n");
        output_geo_stream << QString("</kml>\n");

        kml_file.close();
    }
}

bool Texturing3D::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // get params
    m_source_dir = absoluteOutputTempDir();

    m_outdir = absoluteOutputTempDir();

    m_out_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    return true;
}

bool Texturing3D::stop()
{
    return true;
}

void Texturing3D::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    // Log
    QString proc_info = logPrefix() + "Texturing started\n";
    emit si_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();

    emit si_processCompletion(0);
    emit si_userInformation("Texturing3D start");

    // Get context
    QVariant *object = m_context->getObject("reconstruction_context");
    reconstructionContext * rc;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
        fatalErrorExit("Context issue ...");

    for(unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        // Convert model to mvs-texturing
        emit si_userInformation("Texturing3D convert");
        
        QString scene_dir_i = m_outdir + QDir::separator() + "ModelPart" + QString("_%1").arg(rc->components_ids[i]);
        QString mesh_data_file = scene_dir_i + SEP + m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + rc->out_file_suffix + ".ply";
        QString undist_dir_i = scene_dir_i + SEP + "undist_imgs";
        QString sfm_data_file = scene_dir_i + SEP + "sfm_data.bin";

        if (!this->generateCamFile(sfm_data_file, undist_dir_i))
            continue;

        // Texture model
        emit si_userInformation("Texturing3D texturing");

        QString cmd_line = TexreconExe;
        bool ok = false;
        bool keep_unseen_faces = m_matisse_parameters->getBoolParamValue("algo_param", "keep_unseen_faces", ok);
        if(ok && keep_unseen_faces)
            cmd_line += " --keep_unseen_faces";
        cmd_line += " " + undist_dir_i;
        cmd_line += " "+ mesh_data_file;
        cmd_line += " "+ m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + "_texrecon";
        QProcess texture_proc;
        texture_proc.setWorkingDirectory(scene_dir_i);
        texture_proc.start(cmd_line);

        bool initer = false;
        while(texture_proc.waitForReadyRead(-1)){
            if(!isStarted())
            {
                texture_proc.kill();
                fatalErrorExit("texrecon Cancelled");
                return;
            }

            QString output = texture_proc.readAllStandardOutput();

            if(output.contains("Load and prepare mesh:"))
            {
                // init
                emit si_processCompletion((qint8)-1);
                emit si_userInformation("texrecon : Load mesh...");
            }

            if(output.contains("Generating texture views:"))
            {
                emit si_userInformation("texrecon : Gen. texture views...");
            }

            if(output.contains("Building adjacency graph:"))
            {
                emit si_userInformation("texrecon : Build. adj graphs...");
            }

            if(output.contains("View selection:"))
            {
                emit si_userInformation("texrecon : View selection...");
            }

            if(output.contains("\tCalculating face qualities "))
            {
                emit si_userInformation("texrecon : calc face qual...");
            }

            if(output.contains("\tWriting data cost file"))
            {
                initer = true;
                emit si_userInformation("texrecon : writing data cost...");
            }

            if(initer && output.startsWith("\t"))
            {
                // format
                // "    Comp	Iter	Energy		Runtime"
                // "	0000	0	1.10237e+06	0.017"
                QStringList list = output.mid(1).split("\t");
                if(list.count() > 2)
                    emit si_userInformation("texrecon : view sel. iter " + list.at(1));
            }

            if(output.contains("Generating texture patches:"))
            {
                initer = false;
                emit si_userInformation("texrecon : gen text. patches...");
            }

            if(output.contains("Running global seam leveling:"))
            {
                initer = false;
                emit si_userInformation("texrecon : global seam level...");
            }

            if(output.contains("\tAdjusting texture patches"))
            {
                initer = false;
                emit si_userInformation("texrecon : adj text. patches...");
            }

            if(output.contains("Running local seam leveling:"))
            {
                initer = false;
                emit si_userInformation("texrecon : local seam level...");
            }

            if(output.contains("Generating texture atlases:"))
            {
                initer = false;
                emit si_userInformation("texrecon : gen. text. atlas...");
            }

            if(output.contains("\tFinalizing texture atlases"))
            {
                initer = false;
                emit si_userInformation("texrecon : final. text. atlas...");
            }

            if(output.contains("Building objmodel:"))
            {
                initer = false;
                emit si_userInformation("texrecon : build objmodel...");
            }

            qDebug() << output;
        }

        // Write kml associated to model
        writeKml(scene_dir_i, m_out_filename_prefix + QString("_%1").arg(rc->components_ids[i]) + "_texrecon");

        emit si_processCompletion(100);
        emit si_userInformation("Texturing3D end");

    }

    // update suffix
    rc->out_file_suffix = "_texrecon";

    qDebug() << logPrefix() << " took " << timer.elapsed() / 1000.0 << " seconds";

    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);

    // Flush next module port
//    flush(0);

}

} // namespace matisse

