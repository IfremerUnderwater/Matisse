#include "matching_3d.h"
#include "nav_image.h"

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
#include "nonFree/sift/SIFT_describer_io.hpp"
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

features::EDESCRIBER_PRESET stringToEnum(const QString& sPreset)
{
    features::EDESCRIBER_PRESET preset;
    if (sPreset == "NORMAL")
        preset = features::NORMAL_PRESET;
    else
        if (sPreset == "HIGH")
            preset = features::HIGH_PRESET;
        else
            if (sPreset == "ULTRA")
                preset = features::ULTRA_PRESET;
            else
                preset = features::EDESCRIBER_PRESET(-1);
    return preset;
}


Matching3D::Matching3D() :
    Processor(NULL, "Matching3D", "Match images and filter with geometric transformation", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("algo_param", "force_recompute");
    addExpectedParameter("algo_param", "describer_method");
    addExpectedParameter("algo_param", "describer_preset");
    addExpectedParameter("algo_param", "nearest_matching_method");
    addExpectedParameter("algo_param", "video_mode_matching");
    addExpectedParameter("algo_param", "video_mode_matching_enable");
}

Matching3D::~Matching3D(){

}

bool Matching3D::configure()
{
    return true;
}

void Matching3D::onNewImage(quint32 port, matisse_image::Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool Matching3D::computeFeatures()
{
    static const QString SEP = QDir::separator();

    emit si_processCompletion(0);
    emit si_userInformation("Matching : Compute Features");

    bool ok = true;
    bool force_recompute = m_matisse_parameters->getBoolParamValue("algo_param", "force_recompute", ok);
    bool bUpRight = false;

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();

    // describer method parameter
    QString methodParamval = m_matisse_parameters->getStringParamValue("algo_param", "describer_method");

    // describer preset parameter
    QString presetParamval = m_matisse_parameters->getStringParamValue("algo_param", "describer_preset");

    QString sSfM_Data_Filename = absoluteOutputTempDir() + SEP + "matches" + SEP + "sfm_data.bin";
    QString sOutDir = absoluteOutputTempDir() + SEP + "matches";

    if (sOutDir.isEmpty()) {
        fatalErrorExit("Matching : output dir is empty");
        return false;
    }

    // Create output dir
    if (!stlplus::folder_exists(sOutDir.toStdString()))
    {
        if (!stlplus::folder_create(sOutDir.toStdString()))
        {
            fatalErrorExit("Matching : impossible to create output dir (permissions ?)");
            return false;
        }
    }

    //---------------------------------------
    // a. Load input scene
    //---------------------------------------
    SfM_Data sfm_data;
    if (!Load(sfm_data, sSfM_Data_Filename.toStdString(), ESfM_Data(VIEWS | INTRINSICS))) {
        //std::cerr << std::endl
        //    << "The input file \"" << sSfM_Data_Filename << "\" cannot be read" << std::endl;
        fatalErrorExit("Matcching : input data cannot be read (sfm_data)");
        return false;
    }

    // b. Init the image_describer
    // - retrieve the used one in case of pre-computed features
    // - else create the desired one

    using namespace openMVG::features;
    std::unique_ptr<Image_describer> image_describer;

    const std::string sImage_describer = stlplus::create_filespec(sOutDir.toStdString(), "image_describer", "json");
    if (!force_recompute && stlplus::is_file(sImage_describer))
    {
        // Dynamically load the image_describer from the file (will restore old used settings)
        std::ifstream stream(sImage_describer.c_str());
        if (!stream.is_open())
        {
            fatalErrorExit("Matching : unable to open image describer");
            return false;
        }


        try
        {
            cereal::JSONInputArchive archive(stream);
            archive(cereal::make_nvp("image_describer", image_describer));
        }
        catch (const cereal::Exception& e)
        {
            fatalErrorExit("Matching : unable to open image describer");
            return false;
        }
    }
    else
    {
        // Create the desired Image_describer method.
        // Don't use a factory, perform direct allocation
        if (methodParamval == "SIFT")
        {
            image_describer.reset(new SIFT_Image_describer
            (SIFT_Image_describer::Params(), !bUpRight));
        }
        else
            if (methodParamval == "SIFT_ANATOMY")
            {
                image_describer.reset(
                    new SIFT_Anatomy_Image_describer(SIFT_Anatomy_Image_describer::Params()));
            }
            else
                if (methodParamval == "AKAZE_FLOAT")
                {
                    image_describer = AKAZE_Image_describer::create
                    (AKAZE_Image_describer::Params(AKAZE::Params(), AKAZE_MSURF), !bUpRight);
                }
                else
                    if (methodParamval == "AKAZE_MLDB")
                    {
                        image_describer = AKAZE_Image_describer::create
                        (AKAZE_Image_describer::Params(AKAZE::Params(), AKAZE_MLDB), !bUpRight);
                    }
        if (!image_describer)
        {
            fatalErrorExit("Matching : unable to open image describer");
            return false;
            //std::cerr << "Cannot create the designed Image_describer:"
            //    << methodParamval.toStdString() << "." << std::endl;

        }
        else
        {
            if (!image_describer->Set_configuration_preset(stringToEnum(presetParamval)))
            {
                fatalErrorExit("Matching : unable to set image describer");
                return false;
            }
        }

        // Export the used Image_describer and region type for:
        // - dynamic future regions computation and/or loading
        {
            std::ofstream stream(sImage_describer.c_str());
            if (!stream.is_open())
            {
                fatalErrorExit("Matching : unable to save image describer");
                return false;
            }

            cereal::JSONOutputArchive archive(stream);
            archive(cereal::make_nvp("image_describer", image_describer));
            auto regionsType = image_describer->Allocate();
            archive(cereal::make_nvp("regions_type", regionsType));
        }
    }

    // Feature extraction routines
    // For each View of the SfM_Data container:
    // - if regions file exists continue,
    // - if no file, compute features
    {
        system::Timer timer;
        openMVG::image::Image<unsigned char> imageGray;

        /*C_Progress_display my_progress_bar(sfm_data.GetViews().size(),
            std::cout, "\n- EXTRACT FEATURES -\n");*/
        double total_count = double(sfm_data.GetViews().size());
        std::cout << "total img to extract features from : " << total_count << std::endl;
        int my_progress_bar = 0;

        // Use a boolean to track if we must stop feature extraction
        std::atomic<bool> preemptive_exit(false);

        omp_set_num_threads(nbthreads);

#pragma omp parallel for schedule(dynamic) if (nbthreads > 0) private(imageGray)

        for (int i = 0; i < static_cast<int>(sfm_data.views.size()); ++i)
        {
            Views::const_iterator iterViews = sfm_data.views.begin();
            std::advance(iterViews, i);
            const View* view = iterViews->second.get();
            const std::string
                sView_filename = stlplus::create_filespec(sfm_data.s_root_path, view->s_Img_path),
                sFeat = stlplus::create_filespec(sOutDir.toStdString(), stlplus::basename_part(sView_filename), "feat"),
                sDesc = stlplus::create_filespec(sOutDir.toStdString(), stlplus::basename_part(sView_filename), "desc");

            // If features or descriptors file are missing, compute them
            if (!preemptive_exit && (force_recompute || !stlplus::file_exists(sFeat) || !stlplus::file_exists(sDesc)))
            {
                if (!ReadImage(sView_filename.c_str(), &imageGray))
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

                openMVG::image::Image<unsigned char> imageMask;
                // Try to read the local mask
                if (stlplus::file_exists(mask_filename_local))
                {
                    if (!ReadImage(mask_filename_local.c_str(), &imageMask))
                    {
                        std::cerr << "Invalid mask: " << mask_filename_local << std::endl
                            << "Stopping feature extraction." << std::endl;
                        preemptive_exit = true;
                        continue;
                    }
                    // Use the local mask only if it fits the current image size
                    if (imageMask.Width() == imageGray.Width() && imageMask.Height() == imageGray.Height())
                        mask = &imageMask;
                }
                else
                {
                    // Try to read the global mask
                    if (stlplus::file_exists(mask__filename_global))
                    {
                        if (!ReadImage(mask__filename_global.c_str(), &imageMask))
                        {
                            std::cerr << "Invalid mask: " << mask__filename_global << std::endl
                                << "Stopping feature extraction." << std::endl;
                            preemptive_exit = true;
                            continue;
                        }
                        // Use the global mask only if it fits the current image size
                        if (imageMask.Width() == imageGray.Width() && imageMask.Height() == imageGray.Height())
                            mask = &imageMask;
                    }
                }

                // Compute features and descriptors and export them to files
                auto regions = image_describer->Describe(imageGray, mask);
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

bool Matching3D::computeMatches(EGeometricModel eGeometricModelToCompute)
{
    static const QString SEP = QDir::separator();

    // Dir checks

    bool ok = true;
    bool force_recompute = m_matisse_parameters->getBoolParamValue("algo_param", "force_recompute", ok);

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();

    QString qSfM_Data_Filename = absoluteOutputTempDir() + SEP + "matches" + SEP + "sfm_data.bin";
    QString sOutDir = absoluteOutputTempDir() + SEP + "matches";

    emit si_userInformation("Match 3D : Compute Matches");
    emit si_processCompletion(0);

    // Compute Matches *****************************************************************************************************
 
    // nearest matching method
    QString nmmParamValue = m_matisse_parameters->getStringParamValue("algo_param", "nearest_matching_method");
    
    ok = true;
    int vmmParamVal = m_matisse_parameters->getIntParamValue("algo_param", "video_mode_matching", ok);
    bool video_mode_matching_enable = m_matisse_parameters->getBoolParamValue("algo_param", "video_mode_matching_enable", ok);

    std::string sSfM_Data_Filename = qSfM_Data_Filename.toStdString();
    std::string sMatchesDirectory = sOutDir.toStdString();
    float fDistRatio = 0.8f;
    int iMatchingVideoMode = -1;
    std::string sPredefinedPairList = "";
    std::string sNearestMatchingMethod = nmmParamValue.toStdString();
    bool bGuided_matching = false;
    int imax_iteration = 2048;
    unsigned int ui_max_cache_size = 0;

    // Set matching mode
    if (video_mode_matching_enable)
        iMatchingVideoMode = vmmParamVal;
    EPairMode ePairmode = (iMatchingVideoMode == -1) ? PAIR_EXHAUSTIVE : PAIR_CONTIGUOUS;

    if (sPredefinedPairList.length()) {
        ePairmode = PAIR_FROM_FILE;
        if (iMatchingVideoMode > 0) {
            fatalErrorExit("Matching : incompatible videomode + pairlist");
            //std::cerr << "\nIncompatible options: --videoModeMatching and --pairList" << std::endl;
            return false;
        }
    }

    if (sMatchesDirectory.empty() || !stlplus::is_folder(sMatchesDirectory)) {
        fatalErrorExit("Matching : invalid out dir");
        return false;
    }

    std::string sGeometricMatchesFilename = "";
    switch (eGeometricModelToCompute)
    {
    case FUNDAMENTAL_MATRIX:
        sGeometricMatchesFilename = "matches.f.bin";
        break;
    case ESSENTIAL_MATRIX:
        sGeometricMatchesFilename = "matches.e.bin";
        break;
    case HOMOGRAPHY_MATRIX:
        sGeometricMatchesFilename = "matches.h.bin";
        break;
    case ESSENTIAL_MATRIX_ANGULAR:
        sGeometricMatchesFilename = "matches.f.bin";
        break;
    case ESSENTIAL_MATRIX_ORTHO:
        sGeometricMatchesFilename = "matches.o.bin";
        break;
    case ESSENTIAL_MATRIX_UPRIGHT:
        sGeometricMatchesFilename = "matches.f.bin";
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
    if (!Load(sfm_data, sSfM_Data_Filename, ESfM_Data(VIEWS | INTRINSICS))) {
        std::cerr << std::endl
            << "The input SfM_Data file \"" << sSfM_Data_Filename << "\" cannot be read." << std::endl;
        fatalErrorExit("Matching : Unknown geometric model");
        return false;
    }

    //---------------------------------------
    // Load SfM Scene regions
    //---------------------------------------
    // Init the regions_type from the image describer file (used for image regions extraction)
    using namespace openMVG::features;
    const std::string sImage_describer = stlplus::create_filespec(sMatchesDirectory, "image_describer", "json");
    std::unique_ptr<Regions> regions_type = Init_region_type_from_file(sImage_describer);
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
    C_Progress_display progress;

    if (!regions_provider->load(sfm_data, sMatchesDirectory, regions_type, this)) {
        fatalErrorExit("Matching : invalid regions");
        return false;
    }

    PairWiseMatches map_PutativesMatches;

    // Build some alias from SfM_Data Views data:
    // - List views as a vector of filenames & image sizes
    std::vector<std::string> vec_fileNames;
    std::vector<std::pair<size_t, size_t>> vec_imagesSize;
    {
        vec_fileNames.reserve(sfm_data.GetViews().size());
        vec_imagesSize.reserve(sfm_data.GetViews().size());
        for (Views::const_iterator iter = sfm_data.GetViews().begin();
            iter != sfm_data.GetViews().end();
            ++iter)
        {
            const View* v = iter->second.get();
            vec_fileNames.push_back(stlplus::create_filespec(sfm_data.s_root_path,
                v->s_Img_path));
            vec_imagesSize.push_back(std::make_pair(v->ui_width, v->ui_height));
        }
    }

    std::cout << std::endl << " - PUTATIVE MATCHES - " << std::endl;
    // If the matches already exists, reload them
    if (!force_recompute
        && (stlplus::file_exists(sMatchesDirectory + "/matches.putative.txt")
            || stlplus::file_exists(sMatchesDirectory + "/matches.putative.bin"))
        )
    {
        if (!(Load(map_PutativesMatches, sMatchesDirectory + "/matches.putative.bin") ||
            Load(map_PutativesMatches, sMatchesDirectory + "/matches.putative.txt")))
        {
            fatalErrorExit("Matching : Cannot load input matches file");
            return false;
        }
        std::cout << "\t PREVIOUS RESULTS LOADED;"
            << " #pair: " << map_PutativesMatches.size() << std::endl;
    }
    else // Compute the putative matches
    {
        std::cout << "Use: ";
        switch (ePairmode)
        {
        case PAIR_EXHAUSTIVE: std::cout << "exhaustive pairwise matching" << std::endl; break;
        case PAIR_CONTIGUOUS: std::cout << "sequence pairwise matching" << std::endl; break;
        case PAIR_FROM_FILE:  std::cout << "user defined pairwise matching" << std::endl; break;
        }

        // Allocate the right Matcher according the Matching requested method
        std::unique_ptr<Matcher> collectionMatcher;
        if (sNearestMatchingMethod == "AUTO")
        {
            if (regions_type->IsScalar())
            {
                std::cout << "Using FAST_CASCADE_HASHING_L2 matcher" << std::endl;
                collectionMatcher.reset(new Cascade_Hashing_Matcher_Regions(fDistRatio));
            }
            else
                if (regions_type->IsBinary())
                {
                    std::cout << "Using BRUTE_FORCE_HAMMING matcher" << std::endl;
                    collectionMatcher.reset(new Matcher_Regions(fDistRatio, BRUTE_FORCE_HAMMING));
                }
        }
        else
            if (sNearestMatchingMethod == "BRUTEFORCEL2")
            {
                std::cout << "Using BRUTE_FORCE_L2 matcher" << std::endl;
                collectionMatcher.reset(new Matcher_Regions(fDistRatio, BRUTE_FORCE_L2));
            }
            else
                if (sNearestMatchingMethod == "BRUTEFORCEHAMMING")
                {
                    std::cout << "Using BRUTE_FORCE_HAMMING matcher" << std::endl;
                    collectionMatcher.reset(new Matcher_Regions(fDistRatio, BRUTE_FORCE_HAMMING));
                }
                else
                    if (sNearestMatchingMethod == "HNSWL2")
                    {
                        std::cout << "Using HNSWL2 matcher" << std::endl;
                        collectionMatcher.reset(new Matcher_Regions(fDistRatio, HNSW_L2));
                    }
                    else
                        if (sNearestMatchingMethod == "ANNL2")
                        {
                            std::cout << "Using ANN_L2 matcher" << std::endl;
                            collectionMatcher.reset(new Matcher_Regions(fDistRatio, ANN_L2));
                        }
                        else
                            if (sNearestMatchingMethod == "CASCADEHASHINGL2")
                            {
                                std::cout << "Using CASCADE_HASHING_L2 matcher" << std::endl;
                                collectionMatcher.reset(new Matcher_Regions(fDistRatio, CASCADE_HASHING_L2));
                            }
                            else
                                if (sNearestMatchingMethod == "FASTCASCADEHASHINGL2")
                                {
                                    std::cout << "Using FAST_CASCADE_HASHING_L2 matcher" << std::endl;
                                    collectionMatcher.reset(new Cascade_Hashing_Matcher_Regions(fDistRatio));
                                }
        if (!collectionMatcher)
        {
            fatalErrorExit("Matching : Invalid Nearest Neighbor method");
            return false;
        }
        // Perform the matching
        system::Timer timer;
        {
            // From matching mode compute the pair list that have to be matched:
            Pair_Set pairs;
            switch (ePairmode)
            {
            case PAIR_EXHAUSTIVE: pairs = exhaustivePairs(sfm_data.GetViews().size()); break;
            case PAIR_CONTIGUOUS: pairs = contiguousWithOverlap(sfm_data.GetViews().size(), iMatchingVideoMode); break;
            case PAIR_FROM_FILE:
                if (!loadPairs(sfm_data.GetViews().size(), sPredefinedPairList, pairs))
                {
                    fatalErrorExit("Matching : cannot load pairs from file");
                    return false;
                }
                break;
            }
            // Photometric matching of putative pairs
            collectionMatcher->Match(regions_provider, pairs, map_PutativesMatches, this);
            //---------------------------------------
            //-- Export putative matches
            //---------------------------------------
            if (!Save(map_PutativesMatches, std::string(sMatchesDirectory + "/matches.putative.bin")))
            {
                fatalErrorExit("Matching : Cannot save computed matches (permissions ?)");
                return false;
            }
        }
        std::cout << "Task (Regions Matching) done in (s): " << timer.elapsed() << std::endl;
    }
    //-- export putative matches Adjacency matrix
    PairWiseMatchingToAdjacencyMatrixSVG(vec_fileNames.size(),
        map_PutativesMatches,
        stlplus::create_filespec(sMatchesDirectory, "PutativeAdjacencyMatrix", "svg"));
    //-- export view pair graph once putative graph matches have been computed
    {
        std::set<IndexT> set_ViewIds;
        std::transform(sfm_data.GetViews().begin(), sfm_data.GetViews().end(),
            std::inserter(set_ViewIds, set_ViewIds.begin()), stl::RetrieveKey());
        graph::indexedGraph putativeGraph(set_ViewIds, getPairs(map_PutativesMatches));
        graph::exportToGraphvizData(
            stlplus::create_filespec(sMatchesDirectory, "putative_matches"),
            putativeGraph);
    }

    //---------------------------------------
    // b. Geometric filtering of putative matches
    //    - AContrario Estimation of the desired geometric model
    //    - Use an upper bound for the a contrario estimated threshold
    //---------------------------------------

    std::unique_ptr<ImageCollectionGeometricFilter> filter_ptr(
        new ImageCollectionGeometricFilter(&sfm_data, regions_provider));

    if (filter_ptr)
    {
        system::Timer timer;
        const double d_distance_ratio = 0.6;

        PairWiseMatches map_GeometricMatches;
        switch (eGeometricModelToCompute)
        {
        case HOMOGRAPHY_MATRIX:
        {
            const bool bGeometric_only_guided_matching = true;
            filter_ptr->Robust_model_estimation(
                GeometricFilter_HMatrix_AC(4.0, imax_iteration),
                map_PutativesMatches, bGuided_matching,
                bGeometric_only_guided_matching ? -1.0 : d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();
        }
        break;
        case FUNDAMENTAL_MATRIX:
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_FMatrix_AC(4.0, imax_iteration),
                map_PutativesMatches, bGuided_matching, d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();
        }
        break;
        case ESSENTIAL_MATRIX:
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_EMatrix_AC(4.0, imax_iteration),
                map_PutativesMatches, bGuided_matching, d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();

            //-- Perform an additional check to remove pairs with poor overlap
            std::vector<PairWiseMatches::key_type> vec_toRemove;
            for (const auto& pairwisematches_it : map_GeometricMatches)
            {
                const size_t putativePhotometricCount = map_PutativesMatches.find(pairwisematches_it.first)->second.size();
                const size_t putativeGeometricCount = pairwisematches_it.second.size();
                const float ratio = putativeGeometricCount / static_cast<float>(putativePhotometricCount);
                if (putativeGeometricCount < 50 || ratio < .3f) {
                    // the pair will be removed
                    vec_toRemove.push_back(pairwisematches_it.first);
                }
            }
            //-- remove discarded pairs
            for (const auto& pair_to_remove_it : vec_toRemove)
            {
                map_GeometricMatches.erase(pair_to_remove_it);
            }
        }
        break;
        case ESSENTIAL_MATRIX_ANGULAR:
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_ESphericalMatrix_AC_Angular<false>(4.0, imax_iteration),
                map_PutativesMatches, bGuided_matching, d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();
        }
        break;
        case ESSENTIAL_MATRIX_ORTHO:
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_EOMatrix_RA(2.0, imax_iteration),
                map_PutativesMatches, bGuided_matching, d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();
        }
        break;
        case ESSENTIAL_MATRIX_UPRIGHT:
        {
            filter_ptr->Robust_model_estimation(
                GeometricFilter_ESphericalMatrix_AC_Angular<true>(4.0, imax_iteration),
                map_PutativesMatches, bGuided_matching, d_distance_ratio, this);
            map_GeometricMatches = filter_ptr->Get_geometric_matches();
        }
        break;
        }

        //---------------------------------------
        //-- Export geometric filtered matches
        //---------------------------------------
        if (!Save(map_GeometricMatches,
            std::string(sMatchesDirectory + "/" + sGeometricMatchesFilename)))
        {
            fatalErrorExit("Matching : Cannot save computed matches (permissions ?)");
            return false;
        }

        std::cout << "Task done in (s): " << timer.elapsed() << std::endl;

        // -- export Geometric View Graph statistics
        graph::getGraphStatistics(sfm_data.GetViews().size(), getPairs(map_GeometricMatches));

        //-- export Adjacency matrix
        std::cout << "\n Export Adjacency Matrix of the pairwise's geometric matches"
            << std::endl;
        PairWiseMatchingToAdjacencyMatrixSVG(vec_fileNames.size(),
            map_GeometricMatches,
            stlplus::create_filespec(sMatchesDirectory, "GeometricAdjacencyMatrix", "svg"));

        //-- export view pair graph once geometric filter have been done
        {
            std::set<IndexT> set_ViewIds;
            std::transform(sfm_data.GetViews().begin(), sfm_data.GetViews().end(),
                std::inserter(set_ViewIds, set_ViewIds.begin()), stl::RetrieveKey());
            graph::indexedGraph putativeGraph(set_ViewIds, getPairs(map_GeometricMatches));
            graph::exportToGraphvizData(
                stlplus::create_filespec(sMatchesDirectory, "geometric_matches"),
                putativeGraph);
        }
    }

    emit si_processCompletion(100);
    emit si_userInformation("Match 3D - ended");

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

void Matching3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

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
    flush(0);

}

} // namespace matisse
