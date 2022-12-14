#include "matching_3d.h"
#include "nav_image.h"
#include "reconstruction_context.h"

#include <QProcess>
#include <QElapsedTimer>

#ifndef OPENMVG_USE_OPENMP
#define OPENMVG_USE_OPENMP
#endif


// for Features extraction
#include <cereal/archives/json.hpp>
#include "openMVG/features/akaze/image_describer_akaze_io.hpp"
#include "openMVG/features/sift/SIFT_Anatomy_Image_Describer_io.hpp"
#include "openMVG/features/regions_factory_io.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/system/timer.hpp"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"
#include "openMVG_dependencies/nonFree/sift/SIFT_describer_io.hpp"
#include <cereal/details/helpers.hpp>


// For matching
#include "openMVG/graph/graph.hpp"
#include "openMVG/graph/graph_stats.hpp"
#include "openMVG/features/akaze/image_describer_akaze.hpp"
#include "openMVG/features/descriptor.hpp"
#include "openMVG/features/feature.hpp"
#include "openMVG/matching/indMatch.hpp"
#include "openMVG/matching/indMatch_utils.hpp"
#include "openMVG/matching_image_collection/Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/Cascade_Hashing_Matcher_Regions.hpp"
#include "openMVG/matching_image_collection/GeometricFilter.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_regions_provider_cache.hpp"
#include "openMVG/matching_image_collection/F_ACRobust.hpp"
#include "openMVG/matching_image_collection/E_ACRobust.hpp"
#include "openMVG/matching_image_collection/E_ACRobust_Angular.hpp"
#include "openMVG/matching_image_collection/Eo_Robust.hpp"
#include "openMVG/matching_image_collection/H_ACRobust.hpp"
#include "openMVG/matching_image_collection/Pair_Builder.hpp"
#include "openMVG/matching/pairwiseAdjacencyDisplay.hpp"
#include "openMVG/stl/stl.hpp"

#include "openMVG/matching/matcher_brute_force.hpp"

#include <atomic>
#include <cstdlib>
#include <fstream>
#include <string>
#include <omp.h>

#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

//#include "GlobalUtil.h"

using namespace openMVG;
using namespace openMVG::image;
using namespace openMVG::features;
using namespace openMVG::sfm;
using namespace openMVG::matching;
using namespace openMVG::robust;
using namespace openMVG::matching_image_collection;
using namespace std;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Matching3D, Matching3D)
#endif

namespace matisse {

features::EDESCRIBER_PRESET stringToEnum(const QString& _s_preset)
{
    features::EDESCRIBER_PRESET preset;
    if (_s_preset == "NORMAL")
        preset = features::NORMAL_PRESET;
    else
        if (_s_preset == "HIGH")
            preset = features::HIGH_PRESET;
        else
            if (_s_preset == "ULTRA")
                preset = features::ULTRA_PRESET;
            else
                preset = features::EDESCRIBER_PRESET(-1);
    return preset;
}


Matching3D::Matching3D() :
    Processor(NULL, "Matching3D", "Match images and filter with geometric transformation", 1, 1),
    m_gpu_features(false)
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

    m_pcontext_manager.reset(new OpenGLContextManager());
}

Matching3D::~Matching3D(){
    //delete m_pcontext_manager;
}

bool Matching3D::configure()
{
    return true;
}

void Matching3D::onNewImage(quint32 _port, matisse_image::Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}

bool Matching3D::computeFeatures()
{
    static const QString SEP = QDir::separator();

    emit si_processCompletion(0);
    emit si_userInformation("Matching : Compute Features");

    bool ok = true;
    bool force_recompute = m_matisse_parameters->getBoolParamValue("algo_param", "force_recompute", ok);
    bool b_up_right = false;

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();

    // describer method parameter
    QString method_paramval = m_matisse_parameters->getStringParamValue("algo_param", "describer_method");

    // describer preset parameter
    QString preset_paramval = m_matisse_parameters->getStringParamValue("algo_param", "describer_preset");

    QString s_sfm_data_filename = absoluteOutputTempDir() + SEP + "matches" + SEP + "sfm_data.bin";
    QString s_out_dir = absoluteOutputTempDir() + SEP + "matches";

    if (s_out_dir.isEmpty()) {
        fatalErrorExit("Matching : output dir is empty");
        return false;
    }

    // Create output dir
    if (!stlplus::folder_exists(s_out_dir.toStdString()))
    {
        if (!stlplus::folder_create(s_out_dir.toStdString()))
        {
            fatalErrorExit("Matching : impossible to create output dir (permissions ?)");
            return false;
        }
    }

    //---------------------------------------
    // a. Load input scene
    //---------------------------------------
    SfM_Data sfm_data;
    if (!Load(sfm_data, s_sfm_data_filename.toStdString(), ESfM_Data(VIEWS | INTRINSICS))) {
        fatalErrorExit("Matching : input data cannot be read (sfm_data)");
        return false;
    }

    // b. Init the image_describer
    // - retrieve the used one in case of pre-computed features
    // - else create the desired one
    using namespace openMVG::features;
    std::unique_ptr<Image_describer> image_describer;

    const std::string s_image_describer = stlplus::create_filespec(s_out_dir.toStdString(), "image_describer", "json");
    //if (!force_recompute && stlplus::is_file(s_image_describer))
    //{
    //    // Dynamically load the image_describer from the file (will restore old used settings)
    //    std::ifstream stream(s_image_describer.c_str());
    //    if (!stream.is_open())
    //    {
    //        fatalErrorExit("Matching : unable to open image describer");
    //        return false;
    //    }


    //    try
    //    {
    //        cereal::JSONInputArchive archive(stream);
    //        archive(cereal::make_nvp("image_describer", image_describer));
    //    }
    //    catch (const cereal::Exception& e)
    //    {
    //        fatalErrorExit("Matching : unable to open image describer");
    //        return false;
    //    }
    //}
    //else
    {
		// Create the desired Image_describer method.
		// Don't use a factory, perform direct allocation
		if (method_paramval == "SIFT_GPU")
		{
            image_describer.reset(new GpuSift_Image_describer(GpuSift_Image_describer::Params()));
            m_gpu_features = true;
		}
		else
			if (method_paramval == "SIFT")
			{
				image_describer.reset(new SIFT_Image_describer
				(SIFT_Image_describer::Params(), !b_up_right));
			}
			else
				if (method_paramval == "SIFT_ANATOMY")
				{
					image_describer.reset(
						new SIFT_Anatomy_Image_describer(SIFT_Anatomy_Image_describer::Params()));
				}
				else
					if (method_paramval == "AKAZE_FLOAT")
					{
						image_describer = AKAZE_Image_describer::create
						(AKAZE_Image_describer::Params(AKAZE::Params(), AKAZE_MSURF), !b_up_right);
					}
					else
						if (method_paramval == "AKAZE_MLDB")
						{
							image_describer = AKAZE_Image_describer::create
							(AKAZE_Image_describer::Params(AKAZE::Params(), AKAZE_MLDB), !b_up_right);
						}

        if (!image_describer)
        {
            fatalErrorExit("Matching : unable to open image describer");
            return false;

        }
        else
        {
            if (!image_describer->Set_configuration_preset(stringToEnum(preset_paramval)))
            {
                fatalErrorExit("Matching : unable to set image describer");
                return false;
            }
        }

        // Export the used Image_describer and region type for:
        // - dynamic future regions computation and/or loading
        {
            std::ofstream stream(s_image_describer.c_str());
            if (!stream.is_open())
            {
                fatalErrorExit("Matching : unable to save image describer");
                return false;
            }

            cereal::JSONOutputArchive archive(stream);
            archive(cereal::make_nvp("image_describer", image_describer));
            auto regions_type = image_describer->Allocate();
            archive(cereal::make_nvp("regions_type", regions_type));
        }
    }

    // Feature extraction routines
    // For each View of the SfM_Data container:
    // - if regions file exists continue,
    // - if no file, compute features
    {
        system::Timer timer;
        openMVG::image::Image<unsigned char> image_gray;

        double total_count = double(sfm_data.GetViews().size());
        std::cout << "total img to extract features from : " << total_count << std::endl;
        int my_progress_bar = 0;

        // Use a boolean to track if we must stop feature extraction
        std::atomic<bool> preemptive_exit(false);

        omp_set_num_threads(nbthreads);

#pragma omp parallel for schedule(dynamic) if (nbthreads > 0 && !m_gpu_features) private(image_gray)

        for (int i = 0; i < static_cast<int>(sfm_data.views.size()); ++i)
        {
            Views::const_iterator iter_views = sfm_data.views.begin();
            std::advance(iter_views, i);
            const View* view = iter_views->second.get();
            const std::string
                sView_filename = stlplus::create_filespec(sfm_data.s_root_path, view->s_Img_path),
                sFeat = stlplus::create_filespec(s_out_dir.toStdString(), stlplus::basename_part(sView_filename), "feat"),
                sDesc = stlplus::create_filespec(s_out_dir.toStdString(), stlplus::basename_part(sView_filename), "desc");

            // If features or descriptors file are missing, compute them
            if (!preemptive_exit && (force_recompute || !stlplus::file_exists(sFeat) || !stlplus::file_exists(sDesc)))
            {
                if (!ReadImage(sView_filename.c_str(), &image_gray))
                    continue;

                //
                // Look if there is occlusion feature mask
                //
                openMVG::image::Image<unsigned char>* mask = nullptr; // The mask is null by default

                const std::string
                    mask_filename_local =
                    stlplus::create_filespec(sfm_data.s_root_path,
                        stlplus::basename_part(sView_filename) + "_mask", "png"),
                    mask__filename_global =
                    stlplus::create_filespec(sfm_data.s_root_path, "mask", "png");

                openMVG::image::Image<unsigned char> image_mask;
                // Try to read the local mask
                if (stlplus::file_exists(mask_filename_local))
                {
                    if (!ReadImage(mask_filename_local.c_str(), &image_mask))
                    {
                        std::cerr << "Invalid mask: " << mask_filename_local << std::endl
                            << "Stopping feature extraction." << std::endl;
                        preemptive_exit = true;
                        continue;
                    }
                    // Use the local mask only if it fits the current image size
                    if (image_mask.Width() == image_gray.Width() && image_mask.Height() == image_gray.Height())
                        mask = &image_mask;
                }
                else
                {
                    // Try to read the global mask
                    if (stlplus::file_exists(mask__filename_global))
                    {
                        if (!ReadImage(mask__filename_global.c_str(), &image_mask))
                        {
                            std::cerr << "Invalid mask: " << mask__filename_global << std::endl
                                << "Stopping feature extraction." << std::endl;
                            preemptive_exit = true;
                            continue;
                        }
                        // Use the global mask only if it fits the current image size
                        if (image_mask.Width() == image_gray.Width() && image_mask.Height() == image_gray.Height())
                            mask = &image_mask;
                    }
                }

                // Compute features and descriptors and export them to files
                auto regions = image_describer->Describe(image_gray, mask);

                std::cout << "\n Image  #" << i << " -- nb features : " << regions->RegionCount() << "\n";

                if (regions && !image_describer->Save(regions.get(), sFeat, sDesc)) {
                    std::cerr << "Cannot save regions for images: " << sView_filename << std::endl
                        << "Stopping feature extraction." << std::endl;
                    preemptive_exit = true;
                    continue;
                }
            }
            ++my_progress_bar;
            emit si_processCompletion( (int) (100.0*double(my_progress_bar)/total_count) );
        }
        std::cout << "Task done in (s): " << timer.elapsed() << std::endl;
    }

    return true;
}

bool Matching3D::computeMatches(eGeometricModel _geometric_model_to_compute)
{
    static const QString SEP = QDir::separator();

    // Dir checks

    bool ok = true;
    bool force_recompute = m_matisse_parameters->getBoolParamValue("algo_param", "force_recompute", ok);

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();

    QString q_sfm_data_filename = absoluteOutputTempDir() + SEP + "matches" + SEP + "sfm_data.bin";
    QString s_out_dir = absoluteOutputTempDir() + SEP + "matches";

    emit si_userInformation("Matching : Compute Matches");
    emit si_processCompletion(0);

    // Get context
    QVariant* object = m_context->getObject("reconstruction_context");
    reconstructionContext* rc = nullptr;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
    {
        fatalErrorExit("Matching 3D : Context error");
        return false;
    }

    // Compute Matches *****************************************************************************************************
 
    // nearest matching method (for descriptor matching)
    QString nmm_param_value = m_matisse_parameters->getStringParamValue("algo_param", "nearest_matching_method");
    
    ok = true;

    // Video mode is a transect mode where candidates matching images are selected sequentially
    bool video_mode_matching_enable = m_matisse_parameters->getBoolParamValue("algo_param", "video_mode_matching_enable", ok);
    if (!ok)
        video_mode_matching_enable = false;

    // video_mode_matching defines the number of images to select as matching candidates if video_mode_matching_enable is true
    int vmm_param_val = m_matisse_parameters->getIntParamValue("algo_param", "video_mode_matching", ok);
    if (!ok)
        vmm_param_val = 5;

    // Nav based matching mode uses the prior navigation given for each image in order to select matching candidates
    // based on a spatial distance check (only takes as candidates images closer than a specified max radius from the current image)
    bool nav_based_matching_enable = m_matisse_parameters->getBoolParamValue("algo_param", "nav_based_matching_enable", ok);
    if (!ok)
        nav_based_matching_enable = false;

    // nav_based_matching_max_dist is the maximum radius (distance) at which we consider an images as being a matching candidate
    double nav_based_matching_max_dist = m_matisse_parameters->getDoubleParamValue("algo_param", "nav_based_matching_max_dist", ok);
    if (!ok)
        nav_based_matching_max_dist = 8.0;

    //
    bool b_guided_matching = m_matisse_parameters->getDoubleParamValue("algo_param", "guided_matching", ok);
    if (!ok)
        b_guided_matching = true;

    std::string s_sfm_data_filename = q_sfm_data_filename.toStdString();
    std::string s_matches_directory = s_out_dir.toStdString();
    float f_dist_ratio = 0.8f;
    int i_matching_video_mode = -1;
    std::string s_predefined_pair_list = "";
    std::string s_nearest_matching_method = nmm_param_value.toStdString();
    int imax_iteration = 2048;
    unsigned int ui_max_cache_size = 0;

    // Set matching mode
    ePairMode pair_mode = ePairMode::PAIR_EXHAUSTIVE;

    if (video_mode_matching_enable)
    {
        i_matching_video_mode = vmm_param_val;
        pair_mode = ePairMode::PAIR_CONTIGUOUS;
    }
    else if (nav_based_matching_enable)
    {
        if (rc->all_images_have_nav)
        {
            // Limiting the maximum number of candidates to 50 for the nav based matching mode
            i_matching_video_mode = 50;
            pair_mode = ePairMode::PAIR_FROM_GPS;
        }
        else
            std::cout << "Nav based matching have been required but not all images have navigation -> switching to exhaustive !\n";
    }
        
    if (s_predefined_pair_list.length()) {
        pair_mode = ePairMode::PAIR_FROM_FILE;
        if (i_matching_video_mode > 0) {
            fatalErrorExit("Matching : incompatible videomode + pairlist");
            //std::cerr << "\nIncompatible options: --videoModeMatching and --pairList" << std::endl;
            return false;
        }
    }

    if (s_matches_directory.empty() || !stlplus::is_folder(s_matches_directory)) {
        fatalErrorExit("Matching : invalid out dir");
        return false;
    }

    std::string s_geometric_matches_filename = "";
    switch (_geometric_model_to_compute)
    {
    case FUNDAMENTAL_MATRIX:
        s_geometric_matches_filename = "matches.f.bin";
        break;
    case ESSENTIAL_MATRIX:
        s_geometric_matches_filename = "matches.e.bin";
        break;
    case HOMOGRAPHY_MATRIX:
        s_geometric_matches_filename = "matches.h.bin";
        break;
    case ESSENTIAL_MATRIX_ANGULAR:
        s_geometric_matches_filename = "matches.f.bin";
        break;
    case ESSENTIAL_MATRIX_ORTHO:
        s_geometric_matches_filename = "matches.o.bin";
        break;
    case ESSENTIAL_MATRIX_UPRIGHT:
        s_geometric_matches_filename = "matches.f.bin";
        break;
    default:
        fatalErrorExit("Matching : Unknown geometric model");
        return false;
    }

    // -----------------------------
    // - Load SfM_Data Views & intrinsics data
    // a. Compute putative descriptor matches
    // b. Geometric filtering of putative matches
    // + Export some statistics
    // -----------------------------

    //---------------------------------------
    // Read SfM Scene (image view & intrinsics data)
    //---------------------------------------
    SfM_Data sfm_data;
    if (!Load(sfm_data, s_sfm_data_filename, ESfM_Data(VIEWS | INTRINSICS))) {
        std::cerr << std::endl
            << "The input SfM_Data file \"" << s_sfm_data_filename << "\" cannot be read." << std::endl;
        fatalErrorExit("Matching : Unknown geometric model");
        return false;
    }

    //---------------------------------------
    // Load SfM Scene regions
    //---------------------------------------
    // Init the regions_type from the image describer file (used for image regions extraction)
    using namespace openMVG::features;
    const std::string s_image_describer = stlplus::create_filespec(s_matches_directory, "image_describer", "json");
    std::unique_ptr<Regions> regions_type = Init_region_type_from_file(s_image_describer);
    if (!regions_type)
    {
        fatalErrorExit("Matching : invalid region type file");
        return false;
    }

    //---------------------------------------
    // a. Compute putative descriptor matches
    //    - Descriptor matching (according user method choice)
    //    - Keep correspondences only if NearestNeighbor ratio is ok
    //---------------------------------------

    // Load the corresponding view regions
    std::shared_ptr<Regions_Provider> regions_provider;
    if (ui_max_cache_size == 0)
    {
        // Default regions provider (load & store all regions in memory)
        regions_provider = std::make_shared<Regions_Provider>();
    }
    else
    {
        // Cached regions provider (load & store regions on demand)
        regions_provider = std::make_shared<Regions_Provider_Cache>(ui_max_cache_size);
    }

    // Show the progress on the command line:
    //C_Progress_display progress;

    if (!regions_provider->load(sfm_data, s_matches_directory, regions_type, this)) {
        fatalErrorExit("Matching : invalid regions");
        return false;
    }

    PairWiseMatches map_putatives_matches;

    // Build some alias from SfM_Data Views data:
    // - List views as a vector of filenames & image sizes
    std::vector<std::string> vec_file_names;
    std::vector<std::pair<size_t, size_t>> vec_images_size;
    {
        vec_file_names.reserve(sfm_data.GetViews().size());
        vec_images_size.reserve(sfm_data.GetViews().size());
        for (Views::const_iterator iter = sfm_data.GetViews().begin();
            iter != sfm_data.GetViews().end();
            ++iter)
        {
            const View* v = iter->second.get();
            vec_file_names.push_back(stlplus::create_filespec(sfm_data.s_root_path,
                v->s_Img_path));
            vec_images_size.push_back(std::make_pair(v->ui_width, v->ui_height));
        }
    }

    std::cout << std::endl << " - PUTATIVE MATCHES - " << std::endl;
    // If the matches already exists, reload them
    if (!force_recompute
        && (stlplus::file_exists(s_matches_directory + "/matches.putative.txt")
            || stlplus::file_exists(s_matches_directory + "/matches.putative.bin"))
        )
    {
        if (!(Load(map_putatives_matches, s_matches_directory + "/matches.putative.bin") ||
            Load(map_putatives_matches, s_matches_directory + "/matches.putative.txt")))
        {
            fatalErrorExit("Matching : Cannot load input matches file");
            return false;
        }
        std::cout << "\t PREVIOUS RESULTS LOADED;"
            << " #pair: " << map_putatives_matches.size() << std::endl;
    }
    else // Compute the putative matches
    {
        std::cout << "Use: ";
        switch (pair_mode)
        {
        case ePairMode::PAIR_EXHAUSTIVE: std::cout << "exhaustive pairwise matching" << std::endl; break;
        case ePairMode::PAIR_CONTIGUOUS: std::cout << "sequence pairwise matching" << std::endl; break;
        case ePairMode::PAIR_FROM_FILE:  std::cout << "user defined pairwise matching" << std::endl; break;
        case ePairMode::PAIR_FROM_GPS:  std::cout << "gps based spatial pairwise matching" << std::endl; break;
        }

        // if (pair_mode == ePairMode::PAIR_FROM_GPS)
        // {
        //     // If Spatial Matching Enabled, Apply BF matching!
        //     if (regions_type->IsScalar())
        //         s_nearest_matching_method = "BRUTEFORCEL2";
        //     else
        //         s_nearest_matching_method = "BRUTEFORCEHAMMING";
        // }

        // Allocate the right Matcher according the Matching requested method
        std::unique_ptr<Matcher> collection_matcher;
        if (s_nearest_matching_method == "AUTO")
        {
            if (regions_type->IsScalar())
            {
                std::cout << "Using FAST_CASCADE_HASHING_L2 matcher" << std::endl;
                collection_matcher.reset(new Cascade_Hashing_Matcher_Regions(f_dist_ratio));
            }
            else
                if (regions_type->IsBinary())
                {
                    std::cout << "Using BRUTE_FORCE_HAMMING matcher" << std::endl;
                    collection_matcher.reset(new Matcher_Regions(f_dist_ratio, BRUTE_FORCE_HAMMING));
                }
        }
        else
        if (s_nearest_matching_method == "GPU_BRUTEFORCE")
        {
            std::cout << "Using GPU_BRUTEFORCE matcher" << std::endl;
            collection_matcher.reset(new GpuSift_Matcher_Regions(f_dist_ratio));
        }
        else
        if (s_nearest_matching_method == "BRUTEFORCEL2")
        {
            std::cout << "Using BRUTE_FORCE_L2 matcher" << std::endl;
            collection_matcher.reset(new Matcher_Regions(f_dist_ratio, BRUTE_FORCE_L2));
        }
        else
        if (s_nearest_matching_method == "BRUTEFORCEHAMMING")
        {
            std::cout << "Using BRUTE_FORCE_HAMMING matcher" << std::endl;
            collection_matcher.reset(new Matcher_Regions(f_dist_ratio, BRUTE_FORCE_HAMMING));
        }
        else
        if (s_nearest_matching_method == "HNSWL2")
        {
            std::cout << "Using HNSWL2 matcher" << std::endl;
            collection_matcher.reset(new Matcher_Regions(f_dist_ratio, HNSW_L2));
        }
        else
        if (s_nearest_matching_method == "ANNL2")
        {
            std::cout << "Using ANN_L2 matcher" << std::endl;
            collection_matcher.reset(new Matcher_Regions(f_dist_ratio, ANN_L2));
        }
        else
        if (s_nearest_matching_method == "CASCADEHASHINGL2")
        {
            std::cout << "Using CASCADE_HASHING_L2 matcher" << std::endl;
            collection_matcher.reset(new Matcher_Regions(f_dist_ratio, CASCADE_HASHING_L2));
        }
        else
        if (s_nearest_matching_method == "FASTCASCADEHASHINGL2")
        {
            std::cout << "Using FAST_CASCADE_HASHING_L2 matcher" << std::endl;
            collection_matcher.reset(new Cascade_Hashing_Matcher_Regions(f_dist_ratio));
        }

        if (!collection_matcher)
        {
            fatalErrorExit("Matching : Invalid Nearest Neighbor method");
            return false;
        }
        // Perform the matching
        system::Timer timer;
        {
            // From matching mode compute the pair list that have to be matched:
            Pair_Set pairs;
            switch (pair_mode)
            {
            case ePairMode::PAIR_EXHAUSTIVE: pairs = exhaustivePairs(sfm_data.GetViews().size()); break;
            case ePairMode::PAIR_CONTIGUOUS: pairs = contiguousWithOverlap(sfm_data.GetViews().size(), i_matching_video_mode); break;
            case ePairMode::PAIR_FROM_FILE:
            {
                if (!loadPairs(sfm_data.GetViews().size(), s_predefined_pair_list, pairs))
                {
                    fatalErrorExit("Matching : cannot load pairs from file");
                    return false;
                }
                break;          
            }
            case ePairMode::PAIR_FROM_GPS:
            {
                std::cout << "\n Matching based on PAIR_FROM_GPS \n";

                // List the poses priors
                std::vector<Vec3> vec_pose_centers;
                std::map<IndexT, IndexT> contiguous_to_pose_id;
                std::set<IndexT> used_pose_ids;

                for (const auto & view_it : sfm_data.GetViews())
                {
                    const sfm::ViewPriors * prior = dynamic_cast<sfm::ViewPriors*>(view_it.second.get());

                    if (prior != nullptr && prior->b_use_pose_center_ && used_pose_ids.count(prior->id_pose) == 0)
                    {
                        vec_pose_centers.push_back( prior->pose_center_ );
                        contiguous_to_pose_id[contiguous_to_pose_id.size()] = prior->id_pose;
                        used_pose_ids.insert(prior->id_pose);
                    }
                }

                if (vec_pose_centers.empty())
                {
                    std::cerr << "You are trying to use the gps_mode but your data does"
                    << " not have any pose priors."
                    << std::endl;

                    std::cout << "Setting back to exhaustive matching!" << std::endl;
                    
                    pairs = exhaustivePairs(sfm_data.GetViews().size()); 
                    break;
                }
                
                std::cout << "\n Found #" << vec_pose_centers.size() << " images with GPS coord.! \n";
                

                // Compute i_matching_video_mode neighbor(s) for each pose
                matching::ArrayMatcherBruteForce<double> matcher;

                if (!matcher.Build(vec_pose_centers[0].data(), vec_pose_centers.size(), 3))
                {
                    return EXIT_FAILURE;
                }

                size_t contiguous_pose_id = 0;

                for (const Vec3 pose_it : vec_pose_centers)
                {
                    const double * query = pose_it.data();
                    IndMatches vec_indices;
                    std::vector<double> vec_distance;
                    const int NN = i_matching_video_mode + 1; // since itself will be found    
                    
                    //std::cout << "\n Spatial Matching for Image #" << contiguous_pose_id << " : \n";

                    if (matcher.SearchNeighbours(query, 1, &vec_indices, &vec_distance, NN))
                    {
                        //std::cout << "> Found #" << vec_indices.size() -1 << "neighbor(s) - ids : ";
                        //for (const auto & vec_id : vec_indices)
                        //    std::cout << "(" << vec_id.i_ << "," << vec_id.j_ << "), ";
                        //std::cout << "!\n";

                        // Starting at i=1 because 0 will always be the image itself
                        for (size_t i = 1; i < vec_indices.size(); ++i)
                        {
                            // Do not add pair if images are too spread away
                            if( vec_distance.at(i) > nav_based_matching_max_dist) {
                                //std::cout << "> Images #" << vec_indices[i].j_ << " distance is  : " << vec_distance.at(i) << " m ==> REMOVING PAIR!\n";
                                continue;
                            }

                            IndexT idxI = contiguous_to_pose_id.at(contiguous_pose_id);
                            IndexT idxJ = contiguous_to_pose_id.at(vec_indices[i].j_);
                            if (idxI > idxJ) {
                                std::swap(idxI, idxJ);
                            }
                            pairs.insert(Pair(idxI, idxJ));
                        }
                    }
                    
                    // MAKE SURE THAT Prev & Next views are added as candidates for matching
                    // (safer in case of bad prior nav)
                    if (contiguous_pose_id > 0)
                    {
                        pairs.insert(Pair(IndexT(contiguous_pose_id-1),IndexT(contiguous_pose_id)));
                    }
                    if (size_t(contiguous_pose_id+1) < sfm_data.GetViews().size())
                    {
                        pairs.insert(Pair(IndexT(contiguous_pose_id),IndexT(contiguous_pose_id+1)));
                    }

                    ++contiguous_pose_id;
                }
                
                std::cout << "\n Number of pairs #" << pairs.size() << " : \n";
            }
            break;
            }
            // Photometric matching of putative pairs
            collection_matcher->Match(regions_provider, pairs, map_putatives_matches, this);

            
            collection_matcher.reset();
            //---------------------------------------
            //-- Export putative matches
            //---------------------------------------
            if (!Save(map_putatives_matches, std::string(s_matches_directory + "/matches.putative.bin")))
            {
                fatalErrorExit("Matching : Cannot save computed matches (permissions ?)");
                return false;
            }
        }
        std::cout << "Task (Regions Matching) done in (s): " << timer.elapsed() << std::endl;
    }
    //-- export putative matches Adjacency matrix
    PairWiseMatchingToAdjacencyMatrixSVG(vec_file_names.size(),
        map_putatives_matches,
        stlplus::create_filespec(s_matches_directory, "PutativeAdjacencyMatrix", "svg"));
    //-- export view pair graph once putative graph matches have been computed
    {
        std::set<IndexT> set_view_ids;
        std::transform(sfm_data.GetViews().begin(), sfm_data.GetViews().end(),
            std::inserter(set_view_ids, set_view_ids.begin()), stl::RetrieveKey());
        graph::indexedGraph putative_graph(set_view_ids, getPairs(map_putatives_matches));
        graph::exportToGraphvizData(
            stlplus::create_filespec(s_matches_directory, "putative_matches"),
            putative_graph);
    }

    //---------------------------------------
    // b. Geometric filtering of putative matches
    //    - AContrario Estimation of the desired geometric model
    //    - Use an upper bound for the a contrario estimated threshold
    //---------------------------------------

    std::unique_ptr<GpuImageCollectionGeometricFilter> filter_ptr(
        new GpuImageCollectionGeometricFilter(&sfm_data, regions_provider));
        
    if (filter_ptr)
    {
        system::Timer timer;        
        PairWiseMatches map_geometric_matches;

        // Distance of 2nd best match w.r.t. 1st best match
        // used for guided matching
        const double d_distance_ratio = 0.7; // Optimal value?

        bool use_sift_gpu_4_guided_matching = 
            s_nearest_matching_method == "GPU_BRUTEFORCE";

        // SiftGPU case
        if (use_sift_gpu_4_guided_matching)
        {
            const int max_matches = 4096*4;
            SiftMatchGPU sift_gpu_matcher(max_matches);

            if (sift_gpu_matcher.VerifyContextGL() < 0)
            {
                std::cout << "\n Verify ContexGL returned false!\n";
                std::cout << "Turning back to CPU guided matching if enabled\n";
                use_sift_gpu_4_guided_matching = false;
            }
            else
            {
                const int best_mutual_matches = 1;
                sift_gpu_matcher.Allocate(max_matches, best_mutual_matches);
                
                filter_ptr->Robust_model_estimation(
                    GeometricFilter_H_F_AC(
                        4.0, imax_iteration,
                        &sift_gpu_matcher
                        ),
                    map_putatives_matches, b_guided_matching, 
                    d_distance_ratio,
                    use_sift_gpu_4_guided_matching,
                    this);
            }
        }
        
        // CPU case or SiftGPU failed case
        if(!use_sift_gpu_4_guided_matching)
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_H_F_AC(
                    4.0, imax_iteration
                    ),
                map_putatives_matches, b_guided_matching, 
                d_distance_ratio,
                use_sift_gpu_4_guided_matching, // false here
                this);
        }
        
        // Get resulting new matches
        map_geometric_matches = filter_ptr->Get_geometric_matches();
        
        // switch (_geometric_model_to_compute)
        // {
        // case HOMOGRAPHY_MATRIX:
        // {
        //     const bool b_geometric_only_guided_matching = true;
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_HMatrix_AC(4.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching,
        //         b_geometric_only_guided_matching ? -1.0 : d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();
        // }
        // break;
        // case FUNDAMENTAL_MATRIX:
        // {
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_FMatrix_AC(4.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching, d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();
        // }
        // break;
        // case ESSENTIAL_MATRIX:
        // {
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_EMatrix_AC(4.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching, d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();

        //     //-- Perform an additional check to remove pairs with poor overlap
        //     std::vector<PairWiseMatches::key_type> vec_toRemove;
        //     for (const auto& pairwisematches_it : map_geometric_matches)
        //     {
        //         const size_t putative_photometric_count = map_putatives_matches.find(pairwisematches_it.first)->second.size();
        //         const size_t putative_geometric_count = pairwisematches_it.second.size();
        //         const float ratio = putative_geometric_count / static_cast<float>(putative_photometric_count);
        //         if (putative_geometric_count < 50 || ratio < .3f) {
        //             // the pair will be removed
        //             vec_toRemove.push_back(pairwisematches_it.first);
        //         }
        //     }
        //     //-- remove discarded pairs
        //     for (const auto& pair_to_remove_it : vec_toRemove)
        //     {
        //         map_geometric_matches.erase(pair_to_remove_it);
        //     }
        // }
        // break;
        // case ESSENTIAL_MATRIX_ANGULAR:
        // {
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_ESphericalMatrix_AC_Angular<false>(4.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching, d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();
        // }
        // break;
        // case ESSENTIAL_MATRIX_ORTHO:
        // {
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_EOMatrix_RA(2.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching, d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();
        // }
        // break;
        // case ESSENTIAL_MATRIX_UPRIGHT:
        // {
        //     filter_ptr->Robust_model_estimation(
        //         GeometricFilter_ESphericalMatrix_AC_Angular<true>(4.0, imax_iteration),
        //         map_putatives_matches, b_guided_matching, d_distance_ratio, this);
        //     map_geometric_matches = filter_ptr->Get_geometric_matches();
        // }
        // break;
        // }

        //---------------------------------------
        //-- Export geometric filtered matches
        //---------------------------------------
        if (!Save(map_geometric_matches,
            std::string(s_matches_directory + "/" + s_geometric_matches_filename)))
        {
            fatalErrorExit("Matching : Cannot save computed matches (permissions ?)");
            return false;
        }

        std::cout << "Task done in (s): " << timer.elapsed() << std::endl;

        // -- export Geometric View Graph statistics
        graph::getGraphStatistics(sfm_data.GetViews().size(), getPairs(map_geometric_matches));

        //-- export Adjacency matrix
        std::cout << "\n Export Adjacency Matrix of the pairwise's geometric matches"
            << std::endl;
        PairWiseMatchingToAdjacencyMatrixSVG(vec_file_names.size(),
            map_geometric_matches,
            stlplus::create_filespec(s_matches_directory, "GeometricAdjacencyMatrix", "svg"));

        //-- export view pair graph once geometric filter have been done
        {
            std::set<IndexT> set_view_ids;
            std::transform(sfm_data.GetViews().begin(), sfm_data.GetViews().end(),
                std::inserter(set_view_ids, set_view_ids.begin()), stl::RetrieveKey());
            graph::indexedGraph putative_graph(set_view_ids, getPairs(map_geometric_matches));
            graph::exportToGraphvizData(
                stlplus::create_filespec(s_matches_directory, "geometric_matches"),
                putative_graph);
        }
    }

    emit si_processCompletion(100);
    emit si_userInformation("Matching ended");

    return true;
}

bool Matching3D::start()
{
    setOkStatus();

    return true;
}

bool Matching3D::stop()
{
    return true;
}

void Matching3D::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    // switch opengl context to current processing thread
    m_pcontext_manager->MakeCurrent();

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
