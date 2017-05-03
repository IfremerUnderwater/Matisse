#include "Init3DRecon.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MosaicDescriptor.h"

#include "Polygon.h"



#include "openMVG/exif/exif_IO_EasyExif.hpp"

#include "openMVG/exif/sensor_width_database/ParseDatabase.hpp"
#include "openMVG/exif/exif_IO_EasyExif.hpp"
#include "openMVG/geodesy/geodesy.hpp"

#include "openMVG/image/image.hpp"
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


// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
Q_EXPORT_PLUGIN2(Init3DRecon, Init3DRecon)



/// Check that Kmatrix is a string like "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool checkIntrinsicStringValidity(const std::string & Kmatrix, double & focal, double & ppx, double & ppy)
{
    std::vector<std::string> vec_str;
    stl::split(Kmatrix, ';', vec_str);
    if (vec_str.size() != 9)  {
        std::cerr << "\n Missing ';' character" << std::endl;
        return false;
    }
    // Check that all K matrix value are valid numbers
    for (size_t i = 0; i < vec_str.size(); ++i) {
        double readvalue = 0.0;
        std::stringstream ss;
        ss.str(vec_str[i]);
        if (! (ss >> readvalue) )  {
            std::cerr << "\n Used an invalid not a number character" << std::endl;
            return false;
        }
        if (i==0) focal = readvalue;
        if (i==2) ppx = readvalue;
        if (i==5) ppy = readvalue;
    }
    return true;
}

std::pair<bool, Vec3> checkGPS
(
        const std::string & filename,
        const int & GPS_to_XYZ_method = 0
        )
{
    std::pair<bool, Vec3> val(false, Vec3::Zero());
    std::unique_ptr<Exif_IO> exifReader(new Exif_IO_EasyExif);
    if (exifReader)
    {
        // Try to parse EXIF metada & check existence of EXIF data
        if ( exifReader->open( filename ) && exifReader->doesHaveExifInfo() )
        {
            // Check existence of GPS coordinates
            double latitude, longitude, altitude;
            if ( exifReader->GPSLatitude( &latitude ) &&
                 exifReader->GPSLongitude( &longitude ) &&
                 exifReader->GPSAltitude( &altitude ) )
            {
                // Add ECEF or UTM XYZ position to the GPS position array
                val.first = true;
                switch(GPS_to_XYZ_method)
                {
                case 1:
                    val.second = lla_to_utm( latitude, longitude, altitude );
                    break;
                case 0:
                default:
                    val.second = lla_to_ecef( latitude, longitude, altitude );
                    break;
                }
            }
        }
    }
    return val;
}


/// Check string of prior weights
std::pair<bool, Vec3> checkPriorWeightsString
(
        const std::string &sWeights
        )
{
    std::pair<bool, Vec3> val(true, Vec3::Zero());
    std::vector<std::string> vec_str;
    stl::split(sWeights, ';', vec_str);
    if (vec_str.size() != 3)
    {
        std::cerr << "\n Missing ';' character in prior weights" << std::endl;
        val.first = false;
    }
    // Check that all weight values are valid numbers
    for (size_t i = 0; i < vec_str.size(); ++i)
    {
        double readvalue = 0.0;
        std::stringstream ss;
        ss.str(vec_str[i]);
        if (! (ss >> readvalue) )  {
            std::cerr << "\n Used an invalid not a number character in local frame origin" << std::endl;
            val.first = false;
        }
        val.second[i] = readvalue;
    }
    return val;
}


Init3DRecon::Init3DRecon() :
    Processor(NULL, "Init3DRecon", "Init 2D mosaic Descriptor with navigation", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("cam_param",  "K");
    addExpectedParameter("algo_param", "scale_factor");
    addExpectedParameter("cam_param",  "V_Pose_C");

    addExpectedParameter("algo_param","filter_overlap");
    addExpectedParameter("algo_param","min_overlap");
    addExpectedParameter("algo_param","max_overlap");

}

Init3DRecon::~Init3DRecon(){

}

bool Init3DRecon::configure()
{
    return true;
}

void Init3DRecon::onNewImage(quint32 port, Image &image)
{

    // Forward image
    postImage(0, image);

}

bool Init3DRecon::start()
{
    return true;
}

bool Init3DRecon::stop()
{
    return true;
}

void Init3DRecon::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;

    emit signal_processCompletion(0);
    emit signal_userInformation("Initializing 3D Mosaic...");

    std::string sImageDir,
            sfileDatabase = "",
            sOutputDir = "",
            sKmatrix;

    std::string sPriorWeights;
    std::pair<bool, Vec3> prior_w_info(false, Vec3(1.0,1.0,1.0));

    int i_User_camera_model = PINHOLE_CAMERA_RADIAL3;

    bool b_Group_camera_model = true;
    bool use_prior = true;

    int i_GPS_XYZ_method = 0;

    double focal_pixels = -1.0;


    // Expected properties for each image
    double width = -1, height = -1, focal = -1, ppx = -1,  ppy = -1;

    const EINTRINSIC e_User_camera_model = EINTRINSIC(i_User_camera_model);

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    sImageDir = rootDirnameStr.toStdString();
    sOutputDir = sImageDir + "/matches";

    if ( !stlplus::folder_exists( sImageDir ) )
    {
        std::cerr << "\nThe input directory doesn't exist" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sOutputDir.empty())
    {
        std::cerr << "\nInvalid output directory" << std::endl;
        exit(EXIT_FAILURE);
    }

    if ( !stlplus::folder_exists( sOutputDir ) )
    {
        if ( !stlplus::folder_create( sOutputDir ))
        {
            std::cerr << "\nCannot create output directory" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Check focal length string "f;0;ppx;0;f;ppy;0;0;1"
    bool Ok;
    QMatrix3x3 qK = _matisseParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
    sKmatrix = std::to_string(qK(0,0)) + ";0" + std::to_string(qK(0,2)) + ";0" + std::to_string(qK(1,1)) + ";0" + std::to_string(qK(1,2)) + ";0;0;1";

    if (sKmatrix.size() > 0 &&
            !checkIntrinsicStringValidity(sKmatrix, focal, ppx, ppy) )
    {
        std::cerr << "\nInvalid K matrix input" << std::endl;
        exit(EXIT_FAILURE);
    }

    if (sKmatrix.size() > 0 && focal_pixels != -1.0)
    {
        std::cerr << "\nCannot combine -f and -k options" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::vector<Datasheet> vec_database;
    if (!sfileDatabase.empty())
    {
        if ( !parseDatabase( sfileDatabase, vec_database ) )
        {
            std::cerr
                    << "\nInvalid input database: " << sfileDatabase
                    << ", please specify a valid file." << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    // Check if prior weights are given
    if (!sPriorWeights.empty() && use_prior)
    {
        prior_w_info = checkPriorWeightsString(sPriorWeights);
    }else if (use_prior){
        prior_w_info.first = true;
    }

    std::vector<std::string> vec_image = stlplus::folder_files( sImageDir );
    std::sort(vec_image.begin(), vec_image.end());

    // Configure an empty scene with Views and their corresponding cameras
    SfM_Data sfm_data;
    sfm_data.s_root_path = sImageDir; // Setup main image root_path
    Views & views = sfm_data.views;
    Intrinsics & intrinsics = sfm_data.intrinsics;

    C_Progress_display my_progress_bar( vec_image.size(),
                                        std::cout, "\n- Image listing -\n" );
    std::ostringstream error_report_stream;
    int counter=0;
    for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin();
          iter_image != vec_image.end();
          ++iter_image, ++my_progress_bar )
    {
        counter++;
        emit signal_processCompletion((double)counter/(double)vec_image.size());

        // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
        width = height = ppx = ppy = focal = -1.0;

        const std::string sImageFilename = stlplus::create_filespec( sImageDir, *iter_image );
        const std::string sImFilenamePart = stlplus::filename_part(sImageFilename);

        // Test if the image format is supported:
        if (openMVG::image::GetFormat(sImageFilename.c_str()) == openMVG::image::Unknown)
        {
            error_report_stream
                    << sImFilenamePart << ": Unkown image file format." << "\n";
            continue; // image cannot be opened
        }

        if(sImFilenamePart.find("mask.png") != std::string::npos
                || sImFilenamePart.find("_mask.png") != std::string::npos)
        {
            error_report_stream
                    << sImFilenamePart << " is a mask image" << "\n";
            continue;
        }

        openMVG::image::ImageHeader imgHeader;
        if (!openMVG::image::ReadImageHeader(sImageFilename.c_str(), &imgHeader))
            continue; // image cannot be read

        width = imgHeader.width;
        height = imgHeader.height;
        ppx = width / 2.0;
        ppy = height / 2.0;

        std::unique_ptr<Exif_IO> exifReader(new Exif_IO_EasyExif);
        exifReader->open( sImageFilename );

        const bool bHaveValidExifMetadata =
                exifReader->doesHaveExifInfo()
                && !exifReader->getModel().empty();

        // Consider the case where the focal is provided manually
        if ( !bHaveValidExifMetadata || focal_pixels != -1)
        {
            if (sKmatrix.size() > 0) // Known user calibration K matrix
            {
                if (!checkIntrinsicStringValidity(sKmatrix, focal, ppx, ppy))
                    focal = -1.0;
            }
            else // User provided focal length value
                if (focal_pixels != -1 )
                    focal = focal_pixels;
        }
        else // If image contains meta data
        {
            const std::string sCamModel = exifReader->getModel();

            // Handle case where focal length is equal to 0
            if (exifReader->getFocal() == 0.0f)
            {
                error_report_stream
                        << stlplus::basename_part(sImageFilename) << ": Focal length is missing." << "\n";
                focal = -1.0;
            }
            else
                // Create the image entry in the list file
            {
                Datasheet datasheet;
                if ( getInfo( sCamModel, vec_database, datasheet ))
                {
                    // The camera model was found in the database so we can compute it's approximated focal length
                    const double ccdw = datasheet.sensorSize_;
                    focal = std::max ( width, height ) * exifReader->getFocal() / ccdw;
                }
                else
                {
                    error_report_stream
                            << stlplus::basename_part(sImageFilename)
                            << "\" model \"" << sCamModel << "\" doesn't exist in the database" << "\n"
                            << "Please consider add your camera model and sensor width in the database." << "\n";
                }
            }
        }

        // Build intrinsic parameter related to the view
        std::shared_ptr<IntrinsicBase> intrinsic (NULL);

        if (focal > 0 && ppx > 0 && ppy > 0 && width > 0 && height > 0)
        {
            // Create the desired camera type
            switch(e_User_camera_model)
            {
            case PINHOLE_CAMERA:
                intrinsic = std::make_shared<Pinhole_Intrinsic>
                        (width, height, focal, ppx, ppy);
                break;
            case PINHOLE_CAMERA_RADIAL1:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K1>
                        (width, height, focal, ppx, ppy, 0.0); // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_RADIAL3:
                intrinsic = std::make_shared<Pinhole_Intrinsic_Radial_K3>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0);  // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_BROWN:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Brown_T2>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0, 0.0); // setup no distortion as initial guess
                break;
            case PINHOLE_CAMERA_FISHEYE:
                intrinsic =std::make_shared<Pinhole_Intrinsic_Fisheye>
                        (width, height, focal, ppx, ppy, 0.0, 0.0, 0.0, 0.0); // setup no distortion as initial guess
                break;
            default:
                std::cerr << "Error: unknown camera model: " << (int) e_User_camera_model << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        // Build the view corresponding to the image
        const std::pair<bool, Vec3> gps_info = checkGPS(sImageFilename, i_GPS_XYZ_method);
        if (gps_info.first && use_prior)
        {
            ViewPriors v(*iter_image, views.size(), views.size(), views.size(), width, height);

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
                intrinsics[v.id_intrinsic] = intrinsic;
            }

            v.b_use_pose_center_ = true;
            v.pose_center_ = gps_info.second;
            // prior weights
            if (prior_w_info.first == true)
            {
                v.center_weight_ = prior_w_info.second;
            }

            // Add the view to the sfm_container
            views[v.id_view] = std::make_shared<ViewPriors>(v);
        }
        else
        {
            View v(*iter_image, views.size(), views.size(), views.size(), width, height);

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
                intrinsics[v.id_intrinsic] = intrinsic;
            }

            // Add the view to the sfm_container
            views[v.id_view] = std::make_shared<View>(v);
        }
    }

    // Display saved warning & error messages if any.
    if (!error_report_stream.str().empty())
    {
        std::cerr
                << "\nWarning & Error messages:" << std::endl
                << error_report_stream.str() << std::endl;
    }

    // Group camera that share common properties if desired (leads to more faster & stable BA).
    if (b_Group_camera_model)
    {
        GroupSharedIntrinsics(sfm_data);
    }

    // Store SfM_Data views & intrinsic data
    if (!Save(
                sfm_data,
                stlplus::create_filespec( sOutputDir, "sfm_data.json" ).c_str(),
                ESfM_Data(VIEWS|INTRINSICS)))
    {
        exit(EXIT_FAILURE);
    }

    std::cout << std::endl
              << "SfMInit_ImageListing report:\n"
              << "listed #File(s): " << vec_image.size() << "\n"
              << "usable #File(s) listed in sfm_data: " << sfm_data.GetViews().size() << "\n"
              << "usable #Intrinsic(s) listed in sfm_data: " << sfm_data.GetIntrinsics().size() << std::endl;


    // Flush next module port
    flush(0);

}

