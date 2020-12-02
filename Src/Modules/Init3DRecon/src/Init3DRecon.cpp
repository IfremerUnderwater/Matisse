#include "Init3DRecon.h"
#include "NavImage.h"

#include "reconstructioncontext.h"

#include "Polygon.h"

#include "Dim2FileReader.h"
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

typedef enum
{
    DIM2,
    EXIF,
    NONE
}NavMode;

// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Init3DRecon, Init3DRecon)
#endif

/// Get camera matrix "f;0;ppx;0;f;ppy;0;0;1"
/// With f,ppx,ppy as valid numerical value
bool Init3DRecon::getCameraIntrinsics(double & _focal, double & _ppx, double & _ppy, const double &_width, const double &_height, int &_distortion_model, cv::Mat &_dist_coeff)
{

    bool Ok=false;

    CameraInfo camera_equipment =  _matisseParameters->getCamInfoParamValue("cam_param", "camera_equipment", Ok);

    if(Ok)
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
        Dim2FileReader *dim2Reader,
        std::map<QString, int> &dim2FileMap
        )
{
    QString fname = stlplus::filename_part(filename).c_str();

    std::pair<bool, Vec3> val(false, Vec3::Zero());
    if (dim2Reader && dim2Reader->isFileValid())
    {
        std::map<QString, int>::iterator it = dim2FileMap.find(fname);
        if(it != dim2FileMap.end())
        {
            NavInfo nav = dim2Reader->getNavInfo(it->second);
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
    double reproj_std = _matisseParameters->getDoubleParamValue("vehic_param","reproj_std",ok);
    double X_std = _matisseParameters->getDoubleParamValue("vehic_param","X_std",ok);
    double Y_std = _matisseParameters->getDoubleParamValue("vehic_param","Y_std",ok);
    double depth_std = _matisseParameters->getDoubleParamValue("vehic_param","depth_std",ok);

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

void Init3DRecon::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
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

void Init3DRecon::onFlush(quint32 port)
{
    QElapsedTimer timer;
    timer.start();

    reconstructionContext *reconstruction_context = new reconstructionContext();

    // Log
    QString proc_info =  logPrefix() + "Initialize Sfm from data\n";
    emit signal_addToLog(proc_info);

    emit signal_processCompletion(0);
    emit signal_userInformation("Init3DRecon - start");

    std::ostringstream error_report_stream;

    std::pair<bool, Vec3> prior_w_info(false, Vec3(1.0,1.0,1.0));


    bool b_Group_camera_model = true;
    bool Ok=false;
    bool use_prior = _matisseParameters->getBoolParamValue("dataset_param", "usePrior",Ok);
    if(!Ok)
        use_prior = false;
    //int i_GPS_XYZ_method = 0;

    Vec3 firstImagePos = Vec3::Zero();
    Vec3 firstImagePosLC = Vec3::Zero();
    bool firstImage = true;

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
    std::string dataset_dir = rootDirnameStr.toStdString();
    QString QSep = QDir::separator();
    std::string SEP = QSep.toStdString();
    std::string output_dir = dataset_dir + SEP + "matches";

    if ( !stlplus::folder_exists( dataset_dir ) )
    {
        fatalErrorExit("The input directory doesn't exist");
        return;
    }

    // Navigation source
    QString navSource =  _matisseParameters->getStringParamValue("dataset_param", "navSource");
    NavMode navMode = NONE;
    if(navSource == "NO_NAV")
        navMode = NONE;
    else if(navSource == "EXIF")
        navMode = EXIF;
    else if(navSource == "DIM2")
        navMode = DIM2;
    else
        navMode = DIM2;

    QString navigationFile = _matisseParameters->getStringParamValue("dataset_param", "navFile");

    if (navigationFile.isEmpty())
        navigationFile = QString("noNav.dim2");

    std::string dim2FileName;

    if(navigationFile.contains(":/") || navigationFile.startsWith(QSep) )
    {
        // absolute path
        dim2FileName = navigationFile.toStdString();
    }
    else
    {
        dim2FileName = rootDirnameStr.toStdString() + SEP + navigationFile.toStdString();
    }

    if( navMode == DIM2 && !stlplus::file_exists(dim2FileName))
    {
        if( navSource == "DIM2")
        {
            QString msg = "Dim2 file not found " + navigationFile;
            emit signal_showInformationMessage(this->logPrefix(),msg);
            navMode = NONE;
        }
        else
            navMode = EXIF;

        dim2FileName = "";
    }

    std::map<QString, int> dim2FileMap;
    std::unique_ptr<Dim2FileReader> dim2FileReader(new Dim2FileReader(dim2FileName.c_str()));
    if(navMode == DIM2 && dim2FileReader != NULL && dim2FileReader->isFileValid() )
    {
        for(int i=1; i<= dim2FileReader->getNumberOfImages(); i++ )
        {
            dim2FileMap.insert(std::make_pair(dim2FileReader->getImageFilename(i),i));
        }

    }
    else if(navMode == DIM2 && dim2FileReader != NULL && !dim2FileReader->isFileValid() )
    {
        if( navSource == "DIM2")
        {
            QString msg = "Dim2 file invalid " + navigationFile;
            emit signal_showInformationMessage(this->logPrefix(),msg);
            navMode = NONE;
        }
        else
            navMode = EXIF;
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
    for ( std::vector<std::string>::const_iterator iter_image = vec_image.begin(); iter_image != vec_image.end(); ++iter_image)
    {
        counter++;
        emit signal_processCompletion(100.0*(double)counter/(double)vec_image.size());

        // Read meta data to fill camera parameter (w,h,focal,ppx,ppy) fields.
        width = height = ppx = ppy = focal = -1.0;

        const std::string sImageFilename = stlplus::create_filespec( dataset_dir, *iter_image );
        const std::string sImFilenamePart = stlplus::filename_part(sImageFilename);

        // Test if the image format is supported:
        if (openMVG::image::GetFormat(sImageFilename.c_str()) == openMVG::image::Unknown)
        {
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

        std::unique_ptr<Exif_IO> exifReader(new Exif_IO_EasyExif);
        exifReader->open( sImageFilename );

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
        if(navMode == EXIF)
            gps_info = checkGPS(sImageFilename);
        else if(navMode == DIM2)
        {
            gps_info = checkDIM2(sImageFilename, dim2FileReader.get(), dim2FileMap);
        }

        // get first image info for relative shift (in LC)
        if(gps_info.first && firstImage)
        {
            firstImage = false;
            // lat, lon, alt
            firstImagePos = gps_info.second;
            // localcartesian x, y, alt reset
            m_ltp_proj.Reset(gps_info.second[0], gps_info.second[1], gps_info.second[2]);

            // proj first image pos
            double N,E,U;
            m_ltp_proj.Forward(gps_info.second[0], gps_info.second[1], gps_info.second[2], E, N, U);
            firstImagePosLC = Vec3(E, N, U);
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
            v.pose_center_ = gps_info.second - firstImagePosLC;

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
        }
    }

    // Display saved warning & error messages if any.
    if (!error_report_stream.str().empty())
    {
        emit signal_showInformationMessage(this->logPrefix(),error_report_stream.str().c_str());
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

    emit signal_userInformation("Init3DRecon - saving...");
    // Store SfM_Data views & intrinsic data
    if (!Save(  sfm_data,
                stlplus::create_filespec( output_dir, "sfm_data.bin" ).c_str(),
                ESfM_Data(VIEWS|INTRINSICS)))
    {
        fatalErrorExit("Error saving sfm_data.bin");

        return;
    }

    // Add origin to reconstruction context
    reconstruction_context->lat_origin = firstImagePos[0];
    reconstruction_context->lon_origin = firstImagePos[1];
    reconstruction_context->alt_origin = firstImagePos[2];
    reconstruction_context->current_format = ReconFormat::openMVG;
    reconstruction_context->out_file_suffix = QString("");

    QVariant * reconstruction_context_stocker = new QVariant();
    reconstruction_context_stocker->setValue(reconstruction_context);
    _context->addObject("reconstruction_context",reconstruction_context_stocker);

    // Backup first image position to file
    // firstImagePos : en Lat, Lon, Alt
    QString coords = QString::number(firstImagePos[0],'f',6) +";"
            + QString::number(firstImagePos[1],'f',6) + ";"
            + QString::number(firstImagePos[2],'f',3);
    QString geoFileName = stlplus::create_filespec(rootDirnameStr.toStdString(),"georefpos.txt").c_str();
    QFile geoFile(geoFileName);
    if( !geoFile.open(QIODevice::WriteOnly) )
    {
        fatalErrorExit("Error saving " + geoFileName);
    }

    // Save the pos
    QTextStream outputGeoStream(&geoFile);
    outputGeoStream << coords;
    geoFile.close();

    // Log dataset informations
    proc_info = logPrefix();
    proc_info += QString("usable #File(s) listed in sfm_data: %1\nusable #Intrinsic(s) listed in sfm_data: %2")
            .arg(sfm_data.GetViews().size()).arg(sfm_data.GetIntrinsics().size());
    emit signal_addToLog(proc_info);

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

    emit signal_userInformation("Init3DRecon - end");

    // Log elapsed time
    proc_info = logPrefix() + QString(" took %1 seconds\n").arg(timer.elapsed() / 1000.0);
    emit signal_addToLog(proc_info);

    flush(0);
}

