﻿#include "sfm_bundle_adjustment.h"
#include "reconstruction_context.h"
#include <QProcess>
#include <QElapsedTimer>

#include "openMVG/cameras/Camera_Common.hpp"
#include "openMVG/cameras/Cameras_Common_command_line_helper.hpp"
#include "openMVG/sfm/pipelines/sequential/sequential_SfM2.hpp"
#include "openMVG/sfm/pipelines/sequential/SfmSceneInitializerMaxPair.hpp"
#include "openMVG/sfm/pipelines/sequential/SfmSceneInitializerStellar.hpp"
#include "openMVG/sfm/pipelines/sfm_features_provider.hpp"
#include "openMVG/sfm/pipelines/sfm_matches_provider.hpp"
#include "openMVG/sfm/sfm_report.hpp"
#include "openMVG/sfm/sfm_view.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/sfm/sfm_data_graph_utils.hpp"
#include "openMVG/system/timer.hpp"
#include "openMVG/types.hpp"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <utility>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::sfm;

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(SfmBundleAdjustment, SfmBundleAdjustment)
#endif

namespace matisse {

enum class eSfMSceneInitializer
{
    INITIALIZE_EXISTING_POSES,
    INITIALIZE_MAX_PAIR,
    INITIALIZE_AUTO_PAIR,
    INITIALIZE_STELLAR
};

bool StringToEnum_ESfMSceneInitializer
(
    const std::string& _str,
    eSfMSceneInitializer& _scene_initializer
)
{
    const std::map<std::string, eSfMSceneInitializer> string_to_enum_mapping =
    {
      {"EXISTING_POSE", eSfMSceneInitializer::INITIALIZE_EXISTING_POSES},
      {"MAX_PAIR", eSfMSceneInitializer::INITIALIZE_MAX_PAIR},
      {"AUTO_PAIR", eSfMSceneInitializer::INITIALIZE_AUTO_PAIR},
      {"STELLAR", eSfMSceneInitializer::INITIALIZE_STELLAR},
    };
    auto it = string_to_enum_mapping.find(_str);
    if (it == string_to_enum_mapping.end())
        return false;
    _scene_initializer = it->second;
    return true;
}

SfmBundleAdjustment::SfmBundleAdjustment() :
    Processor(NULL, "SfmBundleAdjustment", "Estimate camera positions and 3D sparse points", 1, 1),
    m_use_prior(true)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "usePrior");
}

SfmBundleAdjustment::~SfmBundleAdjustment(){

}

bool SfmBundleAdjustment::configure()
{
    return true;
}

void SfmBundleAdjustment::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}


bool SfmBundleAdjustment::splitMatchesFiles()
{
    static const QString SEP = QDir::separator();
 
    QDir splitted_matches_path_dir(m_splitted_matches_path);

    // Check if path exists
    if (!splitted_matches_path_dir.exists())
    {
        if ( !splitted_matches_path_dir.mkpath( splitted_matches_path_dir.absolutePath() ) )
        {
            fatalErrorExit("Bundle Adjustment : Cannot create the output directory");
            return false;
        }
    }

    QString match_filename = m_matches_path + SEP + "matches.f.bin";
    QString match_component_filename = splitted_matches_path_dir.absoluteFilePath("matches_list.txt");
    bool is_biedge = false;
    int min_nodes = 3;

    // Load input SfM_Data scene
    SfM_Data sfm_data;
    if (!Load(sfm_data, m_sfm_data_file.toStdString(), ESfM_Data(VIEWS | INTRINSICS)))
    {
        fatalErrorExit("Bundle Adjustment : cannot open sfm_data");
        return false;
    }

    // Matches reading
    std::shared_ptr<Matches_Provider> matches_provider = std::make_shared<Matches_Provider>();
    if (!(matches_provider->load(sfm_data, match_filename.toStdString() )))
    {
        fatalErrorExit("Bundle Adjustment : Invalid matches file");
        return false;
    }

    std::vector<matching::PairWiseMatches> subgraphs_matches;

    // Split match_filename by connected components;
    const bool success_flag =
        SplitMatchesIntoSubgraphMatches(matches_provider->getPairs(),
            matches_provider->pairWise_matches_,
            is_biedge,
            min_nodes,
            subgraphs_matches);
    if (!success_flag)
    {
        fatalErrorExit("Bundle Adjustment : Failed to split matches file into subgraph matches");
        return false;
    }

    // Save all of the subgraphs into match file
    std::set<std::string> set_filenames;

    const std::string& file_basename = stlplus::basename_part(match_filename.toStdString() );
    const std::string& output_folder = stlplus::folder_part(match_component_filename.toStdString() );
    const std::string& match_file_extension = stlplus::extension_part(match_filename.toStdString() );
    int index = 0;
    for (const auto& subgraph : subgraphs_matches)
    {
        std::stringstream strstream_subgraph_match_filename;
        strstream_subgraph_match_filename << file_basename
            << "_" << index
            << "_" << subgraph.size() << "." << match_file_extension;
        const std::string& subgraph_match_filename = strstream_subgraph_match_filename.str();
        const std::string& subgraph_match_file = stlplus::create_filespec(output_folder, subgraph_match_filename);

        if (matching::Save(subgraph, subgraph_match_file))
        {
            m_matches_files_list << QString::fromStdString(subgraph_match_filename);
            set_filenames.insert(subgraph_match_filename);
        }
        else
        {
            return false;
        }
        ++index;
    }

    // Save the match file name of subgraph into a match component file
    std::ofstream stream(match_component_filename.toStdString().c_str());
    if (!stream.is_open())
    {
        fatalErrorExit("Bundle Adjustment : Cannot open match component file");
        return false;
    }
    for (const auto& iter_filename : set_filenames)
    {
        stream << iter_filename << std::endl;
    }
    stream.close();

    return true;

}

bool SfmBundleAdjustment::incrementalSfm(QString _out_dir, QString _match_file)
{
 
    // params that should be exposed to Matisse in future version
    std::string s_intrinsic_refinement_options = "ADJUST_ALL";
    // std::string s_sfm_initializer_method = "STELLAR";
    int i_User_camera_model = PINHOLE_CAMERA_RADIAL3;
    bool b_use_motion_priors = true;
    int triangulation_method = static_cast<int>(ETriangulationMethod::DEFAULT);
    int resection_method = static_cast<int>(resection::SolverType::DEFAULT);


    if (!isValid(static_cast<ETriangulationMethod>(triangulation_method))) {
        std::cerr << "\n Invalid triangulation method" << std::endl;
        return false;
    }

    if (!isValid(openMVG::cameras::EINTRINSIC(i_User_camera_model))) {
        std::cerr << "\n Invalid camera type" << std::endl;
        return false;
    }

    const cameras::Intrinsic_Parameter_Type intrinsic_refinement_options =
        cameras::StringTo_Intrinsic_Parameter_Type(s_intrinsic_refinement_options);
    if (intrinsic_refinement_options == static_cast<cameras::Intrinsic_Parameter_Type>(0))
    {
        std::cerr << "Invalid input for the Bundle Adjusment Intrinsic parameter refinement option" << std::endl;
        return false;
    }

    // eSfMSceneInitializer scene_initializer_enum;
    // if (!StringToEnum_ESfMSceneInitializer(s_sfm_initializer_method, scene_initializer_enum))
    // {
    //     std::cerr << "Invalid input for the SfM initializer option" << std::endl;
    //     return false;
    // }

    // Load input SfM_Data scene
    SfM_Data sfm_data;
    if (!Load(sfm_data, m_sfm_data_file.toStdString(), ESfM_Data(VIEWS | INTRINSICS | EXTRINSICS))) {
        fatalErrorExit("Bundle adjustment : cannot open sfm_data file");
        return false;
    }

    // Init the regions_type from the image describer file (used for image regions extraction)
    using namespace openMVG::features;
    const std::string s_image_describer = stlplus::create_filespec(m_matches_path.toStdString(), "image_describer", "json");
    std::unique_ptr<Regions> regions_type = Init_region_type_from_file(s_image_describer);
    if (!regions_type)
    {
        fatalErrorExit("Bundle adjustment : Invalid region type file");
        return false;
    }

    // Features reading
    std::shared_ptr<Features_Provider> feats_provider = std::make_shared<Features_Provider>();
    if (!feats_provider->load(sfm_data, m_matches_path.toStdString(), regions_type)) {
        std::cerr << std::endl
            << "Invalid features." << std::endl;
        return false;
    }
    // Matches reading
    std::shared_ptr<Matches_Provider> matches_provider = std::make_shared<Matches_Provider>();
    if ( !(matches_provider->load(sfm_data, stlplus::create_filespec(m_splitted_matches_path.toStdString(), _match_file.toStdString() ))) )
    {
        std::cerr << std::endl
            << "Invalid matches file." << std::endl;
        return false;
    }

    if (_out_dir.isEmpty()) {
        std::cerr << "\nIt is an invalid output directory" << std::endl;
        return false;
    }

    if (!stlplus::folder_exists(_out_dir.toStdString() ))
    {
        if (!stlplus::folder_create( _out_dir.toStdString() ))
        {
            std::cerr << "\nCannot create the output directory" << std::endl;
            return false;
        }
    }


    //---------------------------------------
    // Sequential reconstruction process
    //---------------------------------------

    openMVG::system::Timer timer;

    // std::unique_ptr<SfMSceneInitializer> scene_initializer;
    // switch (scene_initializer_enum)
    // {
    // case eSfMSceneInitializer::INITIALIZE_AUTO_PAIR:
    //     std::cerr << "Not yet implemented." << std::endl;
    //     return false;
    //     break;
    // case eSfMSceneInitializer::INITIALIZE_MAX_PAIR:
    //     scene_initializer.reset(new SfMSceneInitializerMaxPair(sfm_data,
    //         feats_provider.get(),
    //         matches_provider.get()));
    //     break;
    // case eSfMSceneInitializer::INITIALIZE_EXISTING_POSES:
    //     scene_initializer.reset(new SfMSceneInitializer(sfm_data,
    //         feats_provider.get(),
    //         matches_provider.get()));
    //     break;
    // case eSfMSceneInitializer::INITIALIZE_STELLAR:
    //     scene_initializer.reset(new SfMSceneInitializerStellar(sfm_data,
    //         feats_provider.get(),
    //         matches_provider.get()));
    //     break;
    // default:
    //     return false;
    // }
    // if (!scene_initializer)
    // {
    //     std::cerr << "Invalid scene initializer." << std::endl;
    //     return false;
    // }
    
    std::unique_ptr<SfMSceneInitializer> scene_initializer_stellar, scene_initializer_max_pair;
    scene_initializer_stellar = std::make_unique<SfMSceneInitializerStellar>(
                                                    sfm_data,
                                                    feats_provider.get(),
                                                    matches_provider.get());

    scene_initializer_max_pair = std::make_unique<SfMSceneInitializerMaxPair>(
                                                    sfm_data,
                                                    feats_provider.get(),
                                                    matches_provider.get());

    // 1st Try SfM With the Stellar Initialization
    // ============================================
    std::unique_ptr<SequentialSfMReconstructionEngine2> psfm_engine = 
                                        std::make_unique<SequentialSfMReconstructionEngine2>(
                                                scene_initializer_stellar.get(),
                                                sfm_data,
                                                _out_dir.toStdString(),
                                                stlplus::create_filespec(_out_dir.toStdString(), "Reconstruction_Report.html"));

    // Configure the features_provider & the matches_provider
    psfm_engine->SetFeaturesProvider(feats_provider.get());
    psfm_engine->SetMatchesProvider(matches_provider.get());

    // Configure reconstruction parameters
    psfm_engine->Set_Intrinsics_Refinement_Type(intrinsic_refinement_options);
    psfm_engine->SetUnknownCameraType(EINTRINSIC(i_User_camera_model));
    psfm_engine->Set_Use_Motion_Prior(m_use_prior);
    psfm_engine->SetTriangulationMethod(static_cast<ETriangulationMethod>(triangulation_method));
    psfm_engine->SetResectionMethod(static_cast<resection::SolverType>(resection_method));

    // // Run SfM with Stellar Init!
    // // ==========================
    bool sfm_success = psfm_engine->Process();

    if (!sfm_success)
    {
        // Stellar init failed... 
        std::cout << "\n\nStellar based Init Failed...\n==> Trying Max_Pair based init!\n\n";

        // Run SfM with MAX_PAIR Init!
        // ==========================
        psfm_engine.reset( new SequentialSfMReconstructionEngine2(
            scene_initializer_max_pair.get(),
            sfm_data,
            _out_dir.toStdString(),
            stlplus::create_filespec(_out_dir.toStdString(), "Reconstruction_Report.html")));

        // Configure the features_provider & the matches_provider
        psfm_engine->SetFeaturesProvider(feats_provider.get());
        psfm_engine->SetMatchesProvider(matches_provider.get());

        // Configure reconstruction parameters
        psfm_engine->Set_Intrinsics_Refinement_Type(intrinsic_refinement_options);
        psfm_engine->SetUnknownCameraType(EINTRINSIC(i_User_camera_model));
        psfm_engine->Set_Use_Motion_Prior(m_use_prior);
        psfm_engine->SetTriangulationMethod(static_cast<ETriangulationMethod>(triangulation_method));
        psfm_engine->SetResectionMethod(static_cast<resection::SolverType>(resection_method));

        // Run SfM with Stellar Init!
        sfm_success = psfm_engine->Process();
    }

    if (sfm_success)
    {
        std::cout << std::endl << " Total Ac-Sfm took (s): " << timer.elapsed() << std::endl;

        std::cout << "...Generating SfM_Report.html" << std::endl;
        Generate_SfM_Report(psfm_engine->Get_SfM_Data(),
            stlplus::create_filespec(_out_dir.toStdString(), "SfMReconstruction_Report.html"));

        //-- Export to disk computed scene (data & visualizable results)
        std::cout << "...Export SfM_Data to disk." << std::endl;
        Save(psfm_engine->Get_SfM_Data(),
            stlplus::create_filespec(_out_dir.toStdString(), "sfm_data", ".bin"),
            ESfM_Data(ALL));

        Save(psfm_engine->Get_SfM_Data(),
            stlplus::create_filespec(_out_dir.toStdString(), "cloud_and_poses", ".ply"),
            ESfM_Data(ALL));

        // Compute residual sfm to nav error
        std::vector<Vec3> X_SfM, X_GPS;
        std::vector<std::string> X_img_path;

        const int nb_img_registered = psfm_engine->Get_SfM_Data().GetPoses().size();
        
        X_SfM.reserve(nb_img_registered);
        X_GPS.reserve(nb_img_registered);
        X_img_path.reserve(nb_img_registered);
        
        for (const auto& view_it : psfm_engine->Get_SfM_Data().GetViews())
        {
            const sfm::ViewPriors* prior = dynamic_cast<sfm::ViewPriors*>(view_it.second.get());
            if (prior != nullptr && prior->b_use_pose_center_ && psfm_engine->Get_SfM_Data().IsPoseAndIntrinsicDefined(prior))
            {
                X_SfM.push_back(psfm_engine->Get_SfM_Data().GetPoses().at(prior->id_pose).center());
                X_GPS.push_back(prior->pose_center_);
                X_img_path.push_back(view_it.second->s_Img_path);
            }
        }
            
        double rms_error = 0.;
        if (!X_GPS.empty())
        {
            for (size_t i = 0; i < X_GPS.size(); ++i)
            {
                const double err = (X_GPS[i] - X_SfM[i]).squaredNorm();
                rms_error += err;

                if (err > 9.)
                    std::cout << "\n Image " << X_img_path[i] << " has an important error w.r.t. to nav -- error: " << err << " m \n";
            }

            rms_error = std::sqrt(rms_error / static_cast<double>(X_GPS.size()));
            QString proc_info = QString("Model #%1 -- RMS error between nav and sfm = %2 m").arg(m_model_idx+1).arg(rms_error);
            emit si_addToLog(proc_info);

            if (rms_error > 2.)
            {
                emit si_showInformationMessage(tr("Model #%1 -- RMS error too high").arg(m_model_idx+1),tr("The RMS error between optical navigation and vehicle navigation is quite high (%1 m) for Model #%2. You should double check the vehicle navigation.").arg(rms_error).arg(m_model_idx+1));
            }
        }

        QString proc_info = QString("Model #%1 : %2 images reconstructed\n").arg(m_model_idx+1).arg(nb_img_registered);
        emit si_addToLog(proc_info);

        // emit si_showInformationMessage(tr("Number of Reconstructed Images for model #%1").arg(m_model_idx+1),tr("Number of images successfully reconstructed %1.").arg(X_SfM.size()));

        m_nb_img_registered += nb_img_registered;
        if (m_nb_img_tot == 0)
            m_nb_img_tot = (int)psfm_engine->Get_SfM_Data().GetViews().size();

        return true;
    }
    return false;

}

bool SfmBundleAdjustment::start()
{
    setOkStatus();
    m_start_time = QDateTime::currentDateTime();
    m_last_ply_time = m_start_time;

    static const QString SEP = QDir::separator();

    // Get flags
    bool ok;
    m_use_prior = m_matisse_parameters->getBoolParamValue("dataset_param", "usePrior", ok);
    if (!ok)
        m_use_prior = true;

    // Get Paths
    m_matches_path = absoluteOutputTempDir() + SEP + "matches";
    m_splitted_matches_path = absoluteOutputTempDir() + SEP + QString("splitted_matches");
    m_sfm_data_file = absoluteOutputTempDir() + SEP + "matches" + SEP + "sfm_data.bin";

    return true;
}

bool SfmBundleAdjustment::stop()
{
    return true;
}

void SfmBundleAdjustment::onFlush(quint32 _port)
{
    Q_UNUSED(_port)

    // Log
    QString proc_info = logPrefix() + "Bundle adjustement started\n";
    emit si_addToLog(proc_info);

    QElapsedTimer timer;
    timer.start();

    static const QString SEP = QDir::separator();
    QDir splitted_matches_path_dir(m_splitted_matches_path);
    QDir matches_path_dir(m_matches_path);

    // Split matches into matches files
    m_matches_files_list.clear();
    if (!this->splitMatchesFiles())
        return;

    // Get context
    QVariant *object = m_context->getObject("reconstruction_context");
    reconstructionContext * rc=nullptr;
    if (object)
        rc = object->value<reconstructionContext*>();
    else
    {
        fatalErrorExit("Bundle Adjustment : Context error");
        return;
    }

    // Reconstruct sparse for each connected components
    m_nb_img_registered = 0;
    m_nb_img_tot = 0;
    for (m_model_idx=0; m_model_idx<m_matches_files_list.size(); ++m_model_idx)
    {
        // Get match file id
        QRegExp match_file_rex(".+_(\\d+)_(\\d+)");

        if (m_matches_files_list[m_model_idx].contains(match_file_rex))
        {
            rc->components_ids.push_back(match_file_rex.cap(1).toInt());
        }
        else
        {
            fatalErrorExit("Bundle Adjustment : Match file not valid");
            return;
        }

        emit si_processCompletion(-1); // cannot monitor percentage
        emit si_userInformation(QString("Reconstruct 3D part %1/%2").arg(m_model_idx+1).arg(m_matches_files_list.size()));

        // Fill out path
        m_out_complete_path_str = absoluteOutputTempDir() + SEP + "ModelPart"+QString("_%1").arg(rc->components_ids[m_model_idx]);

        // Ask for automatic 3D result loading
        emit si_autoAdd3DFileFromFolderOnMainView(m_out_complete_path_str + QDir::separator() + "*.ply");

        // Compute Sfm bundle adjustment
        this->incrementalSfm(m_out_complete_path_str, m_matches_files_list[m_model_idx]);
 
        emit si_processCompletion(100);
    }

    QString sfm_info = QString("Full SfM Reconstructed %1 images reconstructed out of %2").arg(m_nb_img_registered).arg(m_nb_img_tot);
    emit si_addToLog(sfm_info);

    emit si_showInformationMessage(tr("Full SfM Reconstruction"),tr("%1/%2 images successfully reconstructed.\n").arg(m_nb_img_registered).arg(m_nb_img_tot));

    // Stop checking 3D result loading
    emit si_autoAdd3DFileFromFolderOnMainView("");

    // set format
    rc->current_format = ReconFormat::openMVG;

    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);

    // Flush next module port
//    flush(0);

}

} // namespace matisse

