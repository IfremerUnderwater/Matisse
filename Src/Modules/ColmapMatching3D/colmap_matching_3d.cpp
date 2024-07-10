#include "colmap_matching_3d.h"
#include "nav_image.h"
#include "reconstruction_context.h"

#include <QProcess>
#include <QElapsedTimer>

#include "colmap/exe/feature.h"

#include "colmap/controllers/feature_extraction.h"
#include "colmap/controllers/feature_matching.h"
#include "colmap/controllers/image_reader.h"
#include "colmap/controllers/option_manager.h"
#include "colmap/exe/gui.h"
#include "colmap/sensor/models.h"
#include "colmap/util/misc.h"
#include "colmap/util/opengl_utils.h"



#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ColmapMatching3D, ColmapMatching3D)
#endif

using namespace colmap;

namespace matisse {


ColmapMatching3D::ColmapMatching3D() :
    Processor(NULL, "ColmapMatching3D", "Match images and filter with geometric transformation", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("algo_param", "force_recompute");
    addExpectedParameter("algo_param", "describer_method");
    addExpectedParameter("algo_param", "describer_preset");
    addExpectedParameter("algo_param", "nearest_matching_method");
    addExpectedParameter("algo_param", "video_mode_matching");
    addExpectedParameter("algo_param", "video_mode_matching_enable");
    addExpectedParameter("algo_param", "nav_based_matching_enable");
    addExpectedParameter("algo_param", "nav_based_matching_max_dist");
    addExpectedParameter("algo_param", "guided_matching");
    addExpectedParameter("algo_param", "force_gpu_usage");
}

ColmapMatching3D::~ColmapMatching3D()
{}

bool ColmapMatching3D::configure()
{
    return true;
}

void ColmapMatching3D::onNewImage(quint32 _port, matisse_image::Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}

bool ColmapMatching3D::computeFeatures()
{
    // Dir init
    QDir dataset_dir(absoluteDatasetDir());
    QDir output_dir(absoluteOutputTempDir());
    QString qsep = QDir::separator();

    QString filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file = absoluteOutputTempDir() + qsep + filename_prefix + ".db";

    // Todo : handle that in Matisse param ?
    std::string descriptor_normalization = "l1_root";

    bool kUseOpenGL = false;

    OptionManager options;

    ImageReaderOptions reader_options = *options.image_reader;
    reader_options.database_path = database_file.toStdString();
    reader_options.image_path = absoluteDatasetDir().toStdString();

    // Todo : handle that in Matisse param ?
    UpdateImageReaderOptionsFromCameraMode(reader_options,
            CameraMode::PER_FOLDER);

    StringToLower(&descriptor_normalization);
    if (descriptor_normalization == "l1_root") {
        options.sift_extraction->normalization =
            SiftExtractionOptions::Normalization::L1_ROOT;
    }
    else if (descriptor_normalization == "l2") {
        options.sift_extraction->normalization =
            SiftExtractionOptions::Normalization::L2;
    }
    else {
        fatalErrorExit("Invalid `descriptor_normalization`");
        return false;
    }

    reader_options.camera_model="RADIAL";
    if (!ExistsCameraModelWithName(reader_options.camera_model)) {
        fatalErrorExit("Camera model does not exist");
        return false;
    }

    if (!VerifyCameraParams(reader_options.camera_model,
        reader_options.camera_params)) {
        fatalErrorExit("Some parameters are not valid");
        return false;
    }

    if (!VerifySiftGPUParams(options.sift_extraction->use_gpu)) {
        fatalErrorExit("SiftGpu parameters are not valid");
        return false;
    }

    std::unique_ptr<QApplication> app;
    if (options.sift_extraction->use_gpu && kUseOpenGL) {
        int argc = 0;
        char** argv = nullptr;
        app.reset(new QApplication(argc, argv));
    }

    auto feature_extractor = CreateFeatureExtractorController(
        reader_options, *options.sift_extraction);

    if (options.sift_extraction->use_gpu && kUseOpenGL) {
        RunThreadWithOpenGLContext(feature_extractor.get());
    }
    else {
        feature_extractor->Start();
        feature_extractor->Wait();
    }

    return true;
}

bool ColmapMatching3D::computeMatches()
{
    // Dir init
    QDir dataset_dir(absoluteDatasetDir());
    QDir output_dir(absoluteOutputTempDir());
    QString qsep = QDir::separator();

    QString filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file = absoluteOutputTempDir() + qsep + filename_prefix + ".db";

    OptionManager options;
    bool kUseOpenGL = false;
    *options.database_path = database_file.toStdString();

    if (!VerifySiftGPUParams(options.sift_matching->use_gpu)) {
        fatalErrorExit("SiftGpu parameters are not valid");
        return false;
    }

    std::unique_ptr<QApplication> app;
    if (options.sift_matching->use_gpu && kUseOpenGL) {
        int argc = 0;
        char** argv = nullptr;
        app.reset(new QApplication(argc, argv));
    }

    auto matcher = CreateExhaustiveFeatureMatcher(*options.exhaustive_matching,
        *options.sift_matching,
        *options.two_view_geometry,
        *options.database_path);

    if (options.sift_matching->use_gpu && kUseOpenGL) {
        RunThreadWithOpenGLContext(matcher.get());
    }
    else {
        matcher->Start();
        matcher->Wait();
    }

    return true;
}

bool ColmapMatching3D::start()
{
    setOkStatus();

    return true;
}

bool ColmapMatching3D::stop()
{
    return true;
}

void ColmapMatching3D::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    // switch opengl context to current processing thread
    //std::unique_ptr<OpenGLContextManager> m_pcontext_manager;
    //m_pcontext_manager.reset(new OpenGLContextManager());
    // m_pcontext_manager->MakeCurrent();

    // Log
    QString proc_info = logPrefix() + "Features matching started\n";
    emit si_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    if (!this->computeFeatures())
      return;

    this->computeMatches();

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);


    // Flush next module port
//    flush(0);

}

} // namespace matisse
