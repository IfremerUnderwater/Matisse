#include "colmap_mapper.h"
#include "reconstruction_context.h"

#include <QProcess>
#include <QElapsedTimer>

#include <utility>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#include "colmap/exe/sfm.h"

#include "colmap/controllers/automatic_reconstruction.h"
#include "colmap/controllers/bundle_adjustment.h"
#include "colmap/controllers/hierarchical_mapper.h"
#include "colmap/controllers/option_manager.h"
#include "colmap/estimators/similarity_transform.h"
#include "colmap/exe/gui.h"
#include "colmap/scene/reconstruction.h"
#include "colmap/sfm/observation_manager.h"
#include "colmap/util/misc.h"
#include "colmap/util/opengl_utils.h"
#include "colmap/image/undistortion.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ColmapMapper, ColmapMapper)
#endif

using namespace colmap;

namespace matisse {


ColmapMapper::ColmapMapper() :
    Processor(NULL, "ColmapMapper", "Estimate camera positions and 3D sparse points", 1, 1),
    m_use_prior(true)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "usePrior");
    addExpectedParameter("algo_param", "quality_vs_speed");
    
}

ColmapMapper::~ColmapMapper(){

}

bool ColmapMapper::configure()
{
    return true;
}

void ColmapMapper::onNewImage(quint32 _port, matisse_image::Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}

bool ColmapMapper::sfmMapper()
{
    // Dir init
    QDir dataset_dir(absoluteDatasetDir());
    QDir output_dir(absoluteOutputTempDir());
    QString qsep = QDir::separator();

    QString filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file = absoluteOutputTempDir() + qsep + filename_prefix + ".db";

    QString qoutput_path = absoluteOutputTempDir() + qsep + "sfm";
    std::string output_path = qoutput_path.toStdString();

    OptionManager options;
    *options.image_path = absoluteDatasetDir().toStdString();
    *options.database_path = database_file.toStdString();

    options.mapper->triangulation.ignore_two_view_tracks = false;

    // Give options to viewer
    emit si_configColmapViewer(options);

    if (!ExistsDir(output_path)) {
        if (!output_dir.mkdir("sfm"))
        {
            fatalErrorExit("`output_path` is not a directory.");
            return false;
        }
    }

    auto reconstruction_manager = std::make_shared<ReconstructionManager>();

    IncrementalMapperController mapper(options.mapper,
        *options.image_path,
        *options.database_path,
        reconstruction_manager);

    // In case a new reconstruction is started, write results of individual sub-
// models to as their reconstruction finishes instead of writing all results
// after all reconstructions finished.
    size_t prev_num_reconstructions = 0;

    mapper.AddCallback(
        IncrementalMapperController::LAST_IMAGE_REG_CALLBACK, [&]() {
            // If the number of reconstructions has not changed, the last model
            // was discarded for some reason.
            if (reconstruction_manager->Size() > prev_num_reconstructions) {
                const std::string reconstruction_path = JoinPaths(
                    output_path, std::to_string(prev_num_reconstructions));
                CreateDirIfNotExists(reconstruction_path);
                reconstruction_manager->Get(prev_num_reconstructions)
                    ->Write(reconstruction_path);
                options.Write(JoinPaths(reconstruction_path, "project.ini"));
                prev_num_reconstructions = reconstruction_manager->Size();
            }
        });

    mapper.AddCallback(
        IncrementalMapperController::INITIAL_IMAGE_PAIR_REG_CALLBACK, [&]() {
            size_t rec_idx = reconstruction_manager->Size() - 1;
            emit si_updateColmapViewer(reconstruction_manager->Get(rec_idx));
        });
    mapper.AddCallback(
        IncrementalMapperController::NEXT_IMAGE_REG_CALLBACK, [&]() {
            size_t rec_idx = reconstruction_manager->Size() - 1;
            emit si_updateColmapViewer(reconstruction_manager->Get(rec_idx));
        });
    mapper.AddCallback(
        IncrementalMapperController::LAST_IMAGE_REG_CALLBACK, [&]() {
            size_t rec_idx = reconstruction_manager->Size() - 1;
            emit si_updateColmapViewer(reconstruction_manager->Get(rec_idx));
        });

    //mapper.AddCallback(
    //    ControllerThread<IncrementalMapperController>::FINISHED_CALLBACK,
    //    [this]() {
    //        if (!mapper->IsStopped()) {
    //            action_render_now_->trigger();
    //            action_reconstruction_finish_->trigger();
    //        }
    //        if (mapper->Size() == 0) {
    //            action_reconstruction_reset_->trigger();
    //        }
    //    });

    mapper.Run();

    if (reconstruction_manager->Size() == 0) {
        fatalErrorExit("failed to create sparse model");
        return false;
    }

    return true;
}

bool ColmapMapper::undistortImages(QString &_image_path, QString &_sfmdir, QString &_outdir)
{
    std::string input_path = _sfmdir.toStdString();
    std::string output_path= _outdir.toStdString();
    int num_patch_match_src_images = 20;
    CopyType copy_type;

    UndistortCameraOptions undistort_camera_options;

    OptionManager options;
    *options.image_path = _image_path.toStdString();

    CreateDirIfNotExists(output_path);

    PrintHeading1("Reading reconstruction");
    Reconstruction reconstruction;
    reconstruction.Read(input_path);
    LOG(INFO) << StringPrintf("=> Reconstruction with %d images and %d points",
        reconstruction.NumImages(),
        reconstruction.NumPoints3D());

    std::vector<image_t> image_ids;
    copy_type = CopyType::COPY;

    std::unique_ptr<BaseController> undistorter;
        undistorter =
            std::make_unique<COLMAPUndistorter>(undistort_camera_options,
                reconstruction,
                *options.image_path,
                output_path,
                num_patch_match_src_images,
                copy_type,
                image_ids);
    

    undistorter->Run();

    return true;
}

bool ColmapMapper::start()
{
    setOkStatus();

    static const QString SEP = QDir::separator();

    // Get flags
    bool ok;
    m_use_prior = m_matisse_parameters->getBoolParamValue("dataset_param", "usePrior", ok);
    if (!ok)
        m_use_prior = true;


    return true;
}

bool ColmapMapper::stop()
{
    return true;
}

void ColmapMapper::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    // Log
    QString proc_info = logPrefix() + "Bundle adjustement started\n";
    emit si_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    if (!sfmMapper())
        return;

    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);

    // Dir
    QString qsep = QDir::separator();
    QString qoutput_path = absoluteOutputTempDir() + qsep + "sfm";
    std::string output_path = qoutput_path.toStdString();

    // Fill context for next block
    QDir recons_dir(QString::fromStdString(output_path));
    QVariant* object = m_context->getObject("reconstruction_context");

    reconstructionContext* rc;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
    {
        fatalErrorExit("Reconstruction context not found !");
        return;
    }

    QStringList recons_folders = recons_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    // Push folders idx as recons idx (colmap make idx folders)
    for (int i = 0; i < recons_folders.size();i++) {
        rc->components_ids.push_back(i);
        QString sfmdir = qoutput_path + qsep + QString::number(i);
        QString outdir = absoluteOutputTempDir() + qsep + QString("openmvs_result_%1").arg(i);
        QString img_path = absoluteDatasetDir();
		try {
			undistortImages(img_path, sfmdir, outdir);
		}
		catch (const std::future_error& e) {
			std::cerr << "Future error: " << e.what() << std::endl;
		}
		catch (const std::exception& e) {
			std::cerr << "Standard exception: " << e.what() << std::endl;
		}
		catch (...) {
			std::cerr << "Unknown exception caught!" << std::endl;
		}
    }

    // Flush next module port
//    flush(0);

}

} // namespace matisse

