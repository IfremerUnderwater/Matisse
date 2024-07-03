#include "init_colmap_3d_recon.h"
#include "nav_image.h"

#include "reconstruction_context.h"

#include "Polygon.h"

#include "dim2_file_reader.h"
#include <QElapsedTimer>


#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include "file_utils.h"

#include "colmap/scene/database.h"



using namespace nav_tools;


// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(InitColmap3D, InitColmap3D)
#endif

namespace matisse {




InitColmap3D::InitColmap3D() :
    Processor(NULL, "InitColmap3D", "Init 3D reconstruction database", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "navFile");   // dim2 - défaut OTUS.dim2
    addExpectedParameter("dataset_param", "navSource"); // AUTO, GPS, DIM2, NO_NAV
    addExpectedParameter("dataset_param", "usePrior");

    addExpectedParameter("cam_param",  "camera_equipment");

    addExpectedParameter("vehic_param",  "reproj_std");
    addExpectedParameter("vehic_param",  "X_std");
    addExpectedParameter("vehic_param",  "Y_std");
    addExpectedParameter("vehic_param",  "depth_std");

}

InitColmap3D::~InitColmap3D(){

}

bool InitColmap3D::configure()
{
    return true;
}

void InitColmap3D::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}

bool InitColmap3D::start()
{
    setOkStatus();

    std::cerr << "***********Init3DRecon start" << std::endl;
    return true;
}

bool InitColmap3D::stop()
{
    std::cerr << "\n***********Init3DRecon stop" << std::endl;
    return true;
}

void InitColmap3D::onFlush(quint32 _port)
{
    QElapsedTimer timer;
    timer.start();

    reconstructionContext *reconstruction_context = new reconstructionContext();

    // Log
    QString proc_info =  logPrefix() + "Create Colmap database\n";
    emit si_addToLog(proc_info);

    emit si_processCompletion(0);
    emit si_userInformation("InitColmap3D - start");


    // Dir checks
    QDir dataset_dir (absoluteDatasetDir());
    QDir output_dir(absoluteOutputTempDir());
    QString qsep = QDir::separator();

    QString filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // check input exists
    if ( !dataset_dir.exists())
    {
        fatalErrorExit("The input directory doesn't exist");
        return;
    }

    // check output and try to create it
    if (!output_dir.exists())
    {
        if (!output_dir.mkpath(output_dir.absolutePath()))
        {
                    fatalErrorExit("Cannot create output directory");
                    return;
        }
    }

    // Create colmap database
    QString database_file = absoluteOutputTempDir() + qsep + filename_prefix + ".db";
    try
    {
        colmap::Database sfm_database(database_file.toStdString());
    }
    catch (...)
    {
        fatalErrorExit("Cannot create colmap database");
        return;
    }

    // context for next module
    reconstruction_context->current_format = ReconFormat::colmap;
    reconstruction_context->out_file_suffix = QString("");

    QVariant * reconstruction_context_stocker = new QVariant();
    reconstruction_context_stocker->setValue(reconstruction_context);
    m_context->addObject("reconstruction_context",reconstruction_context_stocker);

 

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);

//    flush(0);
}

} // namespace matisse

