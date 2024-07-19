#include "colmap_matching_3d.h"
#include "nav_image.h"
#include "reconstruction_context.h"

#include "dim2_file_reader.h"

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

#include "file_utils.h"



#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(ColmapMatching3D, ColmapMatching3D)
#endif

using namespace colmap;

namespace matisse {


ColmapMatching3D::ColmapMatching3D() :
    Processor(NULL, "ColmapMatching3D", "Match images and filter with geometric transformation", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("dataset_param", "navFile");   // dim2 - défaut OTUS.dim2
    addExpectedParameter("dataset_param", "navSource"); // AUTO, GPS, DIM2, NO_NAV

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

    QString colmap_db_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file_path = absoluteOutputTempDir() + qsep + colmap_db_filename_prefix + ".db";

    // Todo : handle that in Matisse param ?
    std::string descriptor_normalization = "l1_root";

    bool kUseOpenGL = false;

    OptionManager options;

    ImageReaderOptions reader_options = *options.image_reader;
    reader_options.database_path = database_file_path.toStdString();
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

    QString colmap_db_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file_path = absoluteOutputTempDir() + qsep + colmap_db_filename_prefix + ".db";

    OptionManager options;
    bool kUseOpenGL = false;
    *options.database_path = database_file_path.toStdString();

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

bool ColmapMatching3D::writeNavPriors()
{
    // Dir init
    QDir dataset_dir(absoluteDatasetDir());
    QDir output_dir(absoluteOutputTempDir());
    QString qsep = QDir::separator();

    QString colmap_db_filename_prefix = m_matisse_parameters->getStringParamValue("dataset_param", "output_filename");

    // colmap database path
    QString database_file_path = absoluteOutputTempDir() + qsep + colmap_db_filename_prefix + ".db";

    // Nav path file
    const QString nav_source =  m_matisse_parameters->getStringParamValue("dataset_param", "navSource");

    // EXIF is automatically handled by Colmap features extractor
    if (nav_source != "DIM2")
    {
        const QString msg = nav_source == "EXIF" ?
                            "EXIF navSource set -- Nav priors should have been written automatically from EXIF in images."
                            : "NO_NAV navSource set -- No nav priors to be used.";
        
        emit si_showInformationMessage(this->logPrefix(),msg);

        return true;
    }

    QString navigation_file = system_tools::FileUtils::resolveUnixPath(
        m_matisse_parameters->getStringParamValue("dataset_param", "navFile"));

    if (navigation_file.isEmpty())
    {
        navigation_file = QString("noNav.dim2");
    }

    std::string dim2_file_name;

    if(navigation_file.contains(":/") || navigation_file.startsWith(qsep) )
    {
        // absolute path
        dim2_file_name = navigation_file.toStdString();
    }
    else
    {
        dim2_file_name = absoluteDatasetDir().toStdString() + qsep.toStdString() + navigation_file.toStdString();
    }

    if (!colmap::ExistsFile(dim2_file_name))
    {
        const QString msg = "Dim2 file invalid " + navigation_file;
        emit si_showInformationMessage(this->logPrefix(),msg);
        return false;
    }

    // Open DIM2 file
    auto pdim2_file_reader = std::make_unique<nav_tools::Dim2FileReader>(dim2_file_name.c_str());

    // Store img filenames & nav info in a hash-map
    std::unordered_map<std::string, NavInfo> img_names_to_nav_info;

    for (int i=0; i < pdim2_file_reader->getNumberOfImages(); ++i)
    {
        img_names_to_nav_info.emplace(pdim2_file_reader->getImageFilename(i).toStdString(), pdim2_file_reader->getNavInfo(i));
    }
    
    // Open Colmap database
    colmap::Database sfm_database(database_file_path.toStdString());

    colmap::DatabaseTransaction database_transaction(&sfm_database);

    // Get all images in database & write nav prior
    std::size_t nb_prior_added = 0;
    for (const auto& db_image : sfm_database.ReadAllImages())
    {
        const std::string img_filename = colmap::GetPathBaseName(db_image.Name());

        const auto it = img_names_to_nav_info.find(img_filename);
        if (it != img_names_to_nav_info.end())
        {
            const Eigen::Vector3d latlonalt(it->second.latitude(),
                                            it->second.longitude(),
                                            -1.*it->second.depth());
            const colmap::PosePrior prior(latlonalt, colmap::PosePrior::CoordinateSystem::WGS84);

            sfm_database.WritePosePrior(db_image.ImageId(), prior);
            ++nb_prior_added;
        }
    }


    const std::string info_msg = "Added prior nav for " + std::to_string(nb_prior_added) + " / " + std::to_string(sfm_database.NumImages()) + " images.";
;
    emit si_showInformationMessage(this->logPrefix(), info_msg.c_str());

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
    {

        return;
    }

    this->writeNavPriors();

    this->computeMatches();

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);


    // Flush next module port
//    flush(0);

}

} // namespace matisse
