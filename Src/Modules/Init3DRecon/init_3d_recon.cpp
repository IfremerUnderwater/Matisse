#include "init_3d_recon.h"
#include "nav_image.h"

#include "reconstruction_context.h"

#include "Polygon.h"

#include "dim2_file_reader.h"
#include <QElapsedTimer>

//#define PROGRESS
//#define PROGRESS_DISPLAY

#define OPENMVG_SFM_SFM_REGIONS_PROVIDER_HPP
#define OPENMVG_SFM_SFM_REGIONS_PROVIDER_CACHE_HPP

//#include "exifreader.hpp"

#include "openMVG/exif/exif_IO_EasyExif.hpp"

//#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"
#include "openMVG/geodesy/geodesy.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/stl/split.hpp"
#include "openMVG/sfm/sfm.hpp"
#include "third_party/stlplus3/filesystemSimplified/file_system.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


using namespace openMVG;
using namespace openMVG::cameras;
using namespace openMVG::exif;
using namespace openMVG::geodesy;
//using namespace openMVG::image;
using namespace openMVG::sfm;
using namespace nav_tools;


// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init3DRecon, Init3DRecon)
#endif

namespace matisse {

typedef enum
{
    DIM2,
    EXIF,
    NONE
} eNavMode;

/// Get camera matrix "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool Init3DRecon::getCameraIntrinsics(double & _focal, double & _ppx, double & _ppy, const double &_width, const double &_height, int &_distortion_model, cv::Mat &_dist_coeff)
{

    bool ok=false;

    CameraInfo camera_equipment =  m_matisse_parameters->getCamInfoParamValue("cam_param", "camera_equipment", ok);

    if(ok)
    {
        // Unkown case
        if (camera_equipment.cameraName()=="Unknown")
        {
            _focal = _width; // approx in case we don't know
            _ppx = _width / 2.0;
            _ppy = _height / 2.0;
            _distortion_model = 1;
            _dist_coeff = cv::Mat(1,5,CV_64F,0.0);
            return true;
        }

        double scaling_factor;

        int full_sensor_width, full_sensor_height;
        camera_equipment.fullSensorSize(full_sensor_width,full_sensor_height);

        cv::Mat K = camera_equipment.K();

        scaling_factor = _width/(double)full_sensor_width;
        _focal = scaling_factor*(K.at<double>(0,0)+K.at<double>(1,1))/2;
        _ppx = scaling_factor*K.at<double>(0,2);
        _ppy = scaling_factor*K.at<double>(1,2);
        _distortion_model = camera_equipment.distortionModel();
        _dist_coeff = camera_equipment.distortionCoeff();
        return true;
    }
    else
    {
        _focal = _width; // approx in case we don't know
        _ppx = _width / 2.0;
        _ppy = _height / 2.0;
        _distortion_model = 1;
        _dist_coeff = cv::Mat(1,5,CV_64F,0.0);
        return true;
    }


}

static std::pair<bool, Vec3> checkGPS(const std::string & filename)
{
    std::pair<bool, Vec3> val(false, Vec3::Zero());
    std::unique_ptr<Exif_IO> exif_reader(new Exif_IO_EasyExif);
    if (exif_reader)
    {
        // Try to parse EXIF metada & check existence of EXIF data
        if ( exif_reader->open( filename ) && exif_reader->doesHaveExifInfo() )
        {
            // Check existence of GPS coordinates
            double latitude, longitude, altitude;
            if ( exif_reader->GPSLatitude( &latitude ) &&
                 exif_reader->GPSLongitude( &longitude ) &&
                 exif_reader->GPSAltitude( &altitude ) )
            {

                // Add position to the GPS position array
                val.first = true;
                val.second = Vec3(latitude, longitude, altitude);
            }
        }
    }
    return val;
}

static std::pair<bool, Vec3> checkDIM2
(
        const std::string & filename,
        Dim2FileReader *dim2_reader,
        std::map<QString, int> &dim2FileMap
        )
{
    QString fname = stlplus::filename_part(filename).c_str();

    std::pair<bool, Vec3> val(false, Vec3::Zero());
    if (dim2_reader && dim2_reader->isFileValid())
    {
        std::map<QString, int>::iterator it = dim2FileMap.find(fname);
        if(it != dim2FileMap.end())
        {
            NavInfo nav = dim2_reader->getNavInfo(it->second);
            val.first = true;
            val.second = Vec3( nav.latitude(), nav.longitude(), -nav.depth()  );
            return val;
        }
    }
    return val;
}

/// Check string of prior weights
std::pair<bool, Vec3> Init3DRecon::getPriorWeights
()
{
    std::pair<bool, Vec3> val(true, Vec3::Zero());
    bool ok=false; // there is always a default value, no need to check ok bool
    double reproj_std = m_matisse_parameters->getDoubleParamValue("vehic_param","reproj_std",ok);
    double X_std = m_matisse_parameters->getDoubleParamValue("vehic_param","X_std",ok);
    double Y_std = m_matisse_parameters->getDoubleParamValue("vehic_param","Y_std",ok);
    double depth_std = m_matisse_parameters->getDoubleParamValue("vehic_param","depth_std",ok);

    val.second[0]=(reproj_std*reproj_std)/(X_std*X_std);
    val.second[1]=(reproj_std*reproj_std)/(Y_std*Y_std);
    val.second[2]=(reproj_std*reproj_std)/(depth_std*depth_std);

    return val;
}


Init3DRecon::Init3DRecon() :
    Processor(NULL, "Init3DRecon", "Init 3D mosaic Descriptor with navigation", 1, 1)
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

Init3DRecon::~Init3DRecon(){

}

bool Init3DRecon::configure()
{
    return true;
}

void Init3DRecon::onNewImage(quint32 _port, Image &_image)
{
    Q_UNUSED(_port)

    // Forward image
    postImage(0, _image);
}

bool Init3DRecon::start()
{
    setOkStatus();

    std::cerr << "***********Init3DRecon start" << std::endl;
    return true;
}

bool Init3DRecon::stop()
{
    std::cerr << "\n***********Init3DRecon stop" << std::endl;
    return true;
}

void Init3DRecon::onFlush(quint32 _port)
{
    QElapsedTimer timer;
    timer.start();

    reconstructionContext *reconstruction_context = new reconstructionContext();

    // Log
    QString proc_info =  logPrefix() + "Initialize Sfm from data\n";
    emit si_addToLog(proc_info);

    emit si_processCompletion(0);
    emit si_userInformation("Init3DRecon - start");

    std::ostringstream error_report_stream;

    std::pair<bool, Vec3> prior_w_info(false, Vec3(1.0,1.0,1.0));


    bool b_Group_camera_model = true;
    bool Ok=false;
    bool use_prior = m_matisse_parameters->getBoolParamValue("dataset_param", "usePrior",Ok);
    if(!Ok)
        use_prior = false;
    //int i_GPS_XYZ_method = 0;

    Vec3 first_image_pos = Vec3::Zero();
    Vec3 first_image_pos_lc = Vec3::Zero();
    bool first_image = true;

    // Dir checks
    QString root_dirname_str = absoluteDatasetDir();
    std::string dataset_dir = root_dirname_str.toStdString();
    QString qsep = QDir::separator();
    std::string sep = qsep.toStdString();
    std::string output_dir = absoluteOutputTempDir().toStdString() + sep + "matches";

    if ( !stlplus::folder_exists( dataset_dir ) )
    {
        fatalErrorExit("The input directory doesn't exist");
        return;
    }

    // Navigation source
    QString nav_source =  m_matisse_parameters->getStringParamValue("dataset_param", "navSource");
    eNavMode nav_mode = NONE;
    if(nav_source == "NO_NAV")
        nav_mode = NONE;
    else if(nav_source == "EXIF")
        nav_mode = EXIF;
    else if(nav_source == "DIM2")
        nav_mode = DIM2;
    else
        nav_mode = DIM2;

    QString navigation_file = m_matisse_parameters->getStringParamValue("dataset_param", "navFile");

    if (navigation_file.isEmpty())
        navigation_file = QString("noNav.dim2");

    std::string dim2_file_name;

    if(navigation_file.contains(":/") || navigation_file.startsWith(qsep) )
    {
        // absolute path
        dim2_file_name = navigation_file.toStdString();
    }
    else
    {
        dim2_file_name = root_dirname_str.toStdString() + sep + navigation_file.toStdString();
    }

    if( nav_mode == DIM2 && !stlplus::file_exists(dim2_file_name))
    {
        if( nav_source == "DIM2")
        {
            QString msg = "Dim2 file not found " + navigation_file;
            emit si_showInformationMessage(this->logPrefix(),msg);
            nav_mode = NONE;
        }
        else
            nav_mode = EXIF;

        dim2_file_name = "";
    }

    std::map<QString, int> dim2_file_map;
    std::unique_ptr<Dim2FileReader> dim2_file_reader(new Dim2FileReader(dim2_file_name.c_str()));
    if(nav_mode == DIM2 && dim2_file_reader != NULL && dim2_file_reader->isFileValid() )
    {
        for(int i=1; i<= dim2_file_reader->getNumberOfImages(); i++ )
        {
            dim2_file_map.insert(std::make_pair(dim2_file_reader->getImageFilename(i),i));
        }

    }
    else if(nav_mode == DIM2 && dim2_file_reader != NULL && !dim2_file_reader->isFileValid() )
    {
        if( nav_source == "DIM2")
        {
            QString msg = "Dim2 file invalid " + navigation_file;
            emit si_showInformationMessage(this->logPrefix(),msg);
            nav_mode = NONE;
        }
        else
            nav_mode = EXIF;
    }

    if (output_dir.empty())
    {
        fatalErrorExit("Invalid output directory");
        return;
    }

    if ( !stlplus::folder_exists( output_dir ) )
    {
        if ( !stlplus::folder_create( output_dir ))
        {
            fatalErrorExit("Cannot create output directory");
            return;
        }
    }

    // Check if prior weights are given
    if (use_prior)
    {
        prior_w_info = getPriorWeights();
    }

    std::vector<std::string> vec_image = stlplus::folder_files( dataset_dir );
    std::sort(vec_image.begin(), vec_image.end());

    // Configure an empty scene with Views and their corresponding cameras
    SfM_Data sfm_data;
    sfm_data.s_root_path = dataset_dir; // Setup main image root_path

    // Expected properties for each image
    double width = -1, height = -1, focal = -1, ppx = -1,  ppy = -1;
    int dist_model=1;
    cv::Mat dist_coeff = cv::Mat(1,5,CV_64F,0.0);

    // Loop on all images to initialize ***************************************************************
    int counter=0;
    bool all_images_have_nav = true;
    for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin(); iter_image != vec_image.end(); ++iter_image)
    {
        counter++;
        emit si_processCompletion(100.0*(double)counter/(double)vec_image.size());

        // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
        width = height = ppx = ppy = focal = -1.0;

        const std::string s_image_filename = stlplus::create_filespec( dataset_dir, *iter_image );
        const std::string s_im_filename_part = stlplus::filename_part(s_image_filename);

        // Test if the image format is supported:
        if (openMVG::image::GetFormat(s_image_filename.c_str()) == openMVG::image::Unknown)
        {
            continue; // image cannot be opened
        }

        if(s_im_filename_part.find("mask.png") != std::string::npos
                || s_im_filename_part.find("_mask.png") != std::string::npos)
        {
            error_report_stream
                    << s_im_filename_part << " is a mask image" << "\n";
            continue;
        }

        openMVG::image::ImageHeader img_header;
        if (!openMVG::image::ReadImageHeader(s_image_filename.c_str(), &img_header))
            continue; // image cannot be read

        width = img_header.width;
        height = img_header.height;

        std::unique_ptr<Exif_IO> exif_reader(new Exif_IO_EasyExif);
        exif_reader->open( s_image_filename );

        // Get camera intrinsics
        getCameraIntrinsics(focal, ppx, ppy, width, height, dist_model, dist_coeff);

        // Build intrinsic parameter related to the view
        std::shared_ptr<IntrinsicBase> intrinsic (NULL);

        if (focal > 0 && ppx > 0 && ppy > 0 && width > 0 && height > 0)
        {
            // Create the desired camera type
            switch(dist_model)
            {
//            case PINHOLE_CAMERA: // do we need undistorted case -> maybe we'll reactivate it
//                intrinsic = std::make_shared<Pinhole_Intrinsic>
//                        (width, height, focal, ppx, ppy);
//                break;
            case 0:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K1>
                        (width, height, focal, ppx, ppy, dist_coeff.at<double>(0,0));
                break;
            case 1:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K3>
                        (width, height, focal, ppx, ppy, dist_coeff.at<double>(0,0), dist_coeff.at<double>(0,1), dist_coeff.at<double>(0,2));
                break;
            case 2:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Brown_T2>
                        (width, height, focal, ppx, ppy, dist_coeff.at<double>(0,0), dist_coeff.at<double>(0,1), dist_coeff.at<double>(0,2), dist_coeff.at<double>(0,3), dist_coeff.at<double>(0,4));
                break;
            case 3:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Fisheye>
                        (width, height, focal, ppx, ppy, dist_coeff.at<double>(0,0), dist_coeff.at<double>(0,1), dist_coeff.at<double>(0,2), dist_coeff.at<double>(0,3));
                break;
            }
        }

        // Build the view corresponding to the image
        // Navigation
        //
        std::pair<bool, Vec3> gps_info(false, Vec3::Zero());
        if(nav_mode == EXIF)
            gps_info = checkGPS(s_image_filename);
        else if(nav_mode == DIM2)
        {
            gps_info = checkDIM2(s_image_filename, dim2_file_reader.get(), dim2_file_map);
        }

        // get first image info for relative shift (in LC)
        if(gps_info.first && first_image)
        {
            first_image = false;
            // lat, lon, alt
            first_image_pos = gps_info.second;
            // localcartesian x, y, alt reset
            m_ltp_proj.Reset(gps_info.second[0], gps_info.second[1], gps_info.second[2]);

            // proj first image pos
            double N,E,U;
            m_ltp_proj.Forward(gps_info.second[0], gps_info.second[1], gps_info.second[2], E, N, U);
            first_image_pos_lc = Vec3(E, N, U);
        }

        if (gps_info.first && use_prior)
        {
            // use nav
            ViewPriors v(*iter_image, sfm_data.views.size(), sfm_data.views.size(), sfm_data.views.size(), width, height);

            // Add intrinsic related to the image (if any)
            if (intrinsic == NULL)
            {
                //Since the view have invalid intrinsic data
                // (export the view, with an invalid intrinsic field value)
                v.id_intrinsic = UndefinedIndexT;
            }
            else
            {
                // Add the defined intrinsic to the sfm_container
                sfm_data.intrinsics[v.id_intrinsic] = intrinsic;
            }

            v.b_use_pose_center_ = true;
            // local cartesian proj
            double N,E,U;
            m_ltp_proj.Forward(gps_info.second[0], gps_info.second[1], gps_info.second[2], E, N, U);
            gps_info.second = Vec3(E, N, U);
            v.pose_center_ = gps_info.second - first_image_pos_lc;

            // prior weights
            if (prior_w_info.first == true)
            {
                v.center_weight_ = prior_w_info.second;
            }

            // Add the view to the sfm_container
            sfm_data.views[v.id_view] = std::make_shared<ViewPriors>(v);
        }
        else
        {
            View v(*iter_image, sfm_data.views.size(), sfm_data.views.size(), sfm_data.views.size(), width, height);

            // Add intrinsic related to the image (if any)
            if (intrinsic == NULL)
            {
                //Since the view have invalid intrinsic data
                // (export the view, with an invalid intrinsic field value)
                v.id_intrinsic = UndefinedIndexT;
            }
            else
            {
                // Add the defined intrinsic to the sfm_container
                sfm_data.intrinsics[v.id_intrinsic] = intrinsic;
            }

            // Add the view to the sfm_container
            sfm_data.views[v.id_view] = std::make_shared<View>(v);

            all_images_have_nav = false;
        }
    }

    // Display saved warning & error messages if any.
    if (!error_report_stream.str().empty())
    {
        emit si_showInformationMessage(this->logPrefix(),error_report_stream.str().c_str());
    }

    // Group camera that share common properties if desired (leads to more faster & stable BA).
    if (b_Group_camera_model)
    {
        //GroupSharedIntrinsics(sfm_data); // Added the code of GroupSharedIntinsics because calling it in the lib was crashing ... (build options ?)
        Views & views = sfm_data.views;
        Intrinsics & intrinsics = sfm_data.intrinsics;

        // Build hash & build a set of the hash in order to maintain unique Ids
        std::set<size_t> hash_index;
        std::vector<size_t> hash_value;

        for (const auto & intrinsic_it : intrinsics)
        {
            const cameras::IntrinsicBase * intrinsicData = intrinsic_it.second.get();
            const size_t hashVal = intrinsicData->hashValue();
            hash_index.insert(hashVal);
            hash_value.push_back(hashVal);
        }

        // From hash_value(s) compute the new index (old to new indexing)
        Hash_Map<IndexT, IndexT> old_new_reindex;
        size_t i = 0;
        for (const auto & intrinsic_it : intrinsics)
        {
            old_new_reindex[intrinsic_it.first] = std::distance(hash_index.cbegin(), hash_index.find(hash_value[i]));
            ++i;
        }
        //--> Save only the required Intrinsics (do not need to keep all the copy)
        Intrinsics intrinsic_updated;
        for (const auto & intrinsic_it : intrinsics)
        {
            intrinsic_updated[old_new_reindex[intrinsic_it.first]] = intrinsics[intrinsic_it.first];
        }
        // Update intrinsics (keep only the necessary ones) -> swapping
        intrinsics.swap(intrinsic_updated);

        // Update views intrinsic IDs (since some intrinsic position have changed in the map)
        for (auto & view_it: views)
        {
            View * v = view_it.second.get();
            // Update the Id only if a corresponding index exists
            if (old_new_reindex.count(v->id_intrinsic))
                v->id_intrinsic = old_new_reindex[v->id_intrinsic];
        }
    }

    emit si_userInformation("Init3DRecon - saving...");
    // Store SfM_Data views & intrinsic data
    if (!Save(  sfm_data,
                stlplus::create_filespec( output_dir, "sfm_data.bin" ).c_str(),
                ESfM_Data(VIEWS|INTRINSICS)))
    {
        fatalErrorExit("Error saving sfm_data.bin");

        return;
    }

    // Add origin to reconstruction context
    reconstruction_context->lat_origin = first_image_pos[0];
    reconstruction_context->lon_origin = first_image_pos[1];
    reconstruction_context->alt_origin = first_image_pos[2];
    reconstruction_context->all_images_have_nav = all_images_have_nav;
    reconstruction_context->current_format = ReconFormat::openMVG;
    reconstruction_context->out_file_suffix = QString("");

    QVariant * reconstruction_context_stocker = new QVariant();
    reconstruction_context_stocker->setValue(reconstruction_context);
    m_context->addObject("reconstruction_context",reconstruction_context_stocker);

    // Backup first image position to file
    // firstImagePos : en Lat, Lon, Alt
    QString coords = QString::number(first_image_pos[0],'f',6) +";"
            + QString::number(first_image_pos[1],'f',6) + ";"
            + QString::number(first_image_pos[2],'f',3);
    QString geo_file_name = stlplus::create_filespec(absoluteOutputDir().toStdString(),"model_origin.txt").c_str();
    QFile geo_file(geo_file_name);
    if( !geo_file.open(QIODevice::WriteOnly) )
    {
        fatalErrorExit("Error saving " + geo_file_name);
    }

    // Save the pos
    QTextStream output_geo_stream(&geo_file);
    output_geo_stream << coords;
    geo_file.close();

    // Log dataset informations
    proc_info = logPrefix();
    proc_info += QString("usable #File(s) listed in sfm_data: %1\nusable #Intrinsic(s) listed in sfm_data: %2")
            .arg(sfm_data.GetViews().size()).arg(sfm_data.GetIntrinsics().size());
    emit si_addToLog(proc_info);

    if(sfm_data.GetViews().size() == 0)
    {
        fatalErrorExit("No valid images found");
        return;
    }

    sfm_data.poses.clear();
    sfm_data.control_points.clear();
    sfm_data.views.clear();
    std::shared_ptr<IntrinsicBase> intrinsic (NULL);
    sfm_data.intrinsics[0] = intrinsic;
    sfm_data.s_root_path = "";

    emit si_userInformation("Init3DRecon - end");

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit si_addToLog(proc_info);

//    flush(0);
}

} // namespace matisse

