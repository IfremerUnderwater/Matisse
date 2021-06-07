#include "mosaic_descriptor.h"
#include <QDebug>
#include <float.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include "raster_georeferencer.h"
#include "Polygon.h"

using namespace cv;
using namespace basic_processing;
using namespace nav_tools;

namespace optical_mapping {

MosaicDescriptor::MosaicDescriptor():
    m_mosaic_origin(0,0,0),
    m_pixelm_size(0,0),m_mosaic_size(0,0), m_utm_hemisphere("UNDEF"),
    m_utm_zone(-1),m_cameras_owner(true), m_is_initialized(false)
{

    m_mosaic_ullr.zeros(1,4,CV_64F);

}

MosaicDescriptor::~MosaicDescriptor()
{
    if (m_cameras_owner){

        if (!m_camera_nodes.isEmpty()){
            foreach (ProjectiveCamera* cam, m_camera_nodes) {
                delete cam;
            }
        }
    }
}

Point3d MosaicDescriptor::mosaicOrigin() const
{
    return m_mosaic_origin;
}

void MosaicDescriptor::setMosaicOrigin(const Point3d &_mosaic_origin)
{
    m_mosaic_origin = _mosaic_origin;
}
Point2d MosaicDescriptor::pixelSize() const
{
    return m_pixelm_size;
}

void MosaicDescriptor::setPixelSize(const Point2d &_pixel_size)
{
    m_pixelm_size = _pixel_size;

    m_hs = (cv::Mat_<double>(3,3) <<
           1.0/m_pixelm_size.x,               0,       0,
           0,               1.0/m_pixelm_size.y,       0,
           0,                              0,       1);
}
Point2d MosaicDescriptor::mosaicSize() const
{
    return m_mosaic_size;
}

void MosaicDescriptor::setMosaicSize(const Point2d &_mosaic_size)
{
    m_mosaic_size = _mosaic_size;
}
QString MosaicDescriptor::utmHemisphere() const
{
    return m_utm_hemisphere;
}

void MosaicDescriptor::setUtmHemisphere(const QString &_utm_hemisphere)
{
    m_utm_hemisphere = _utm_hemisphere;
}
int MosaicDescriptor::utmZone() const
{
    return m_utm_zone;
}

void MosaicDescriptor::setUtmZone(int _utm_zone)
{
    m_utm_zone = _utm_zone;
}
Mat MosaicDescriptor::mosaic_ullr() const
{
    return m_mosaic_ullr;
}

void MosaicDescriptor::setMosaic_ullr(const Mat &_mosaic_ullr)
{
    m_mosaic_ullr = _mosaic_ullr;
}

void MosaicDescriptor::initCamerasAndFrames(QVector<ProjectiveCamera*> _camera_nodes_p, bool _cameras_owner_p)
{
    m_cameras_owner = _cameras_owner_p;

    m_camera_nodes = _camera_nodes_p;
    double mean_lat=0;
    double mean_lon=0;
    double mean_alt=0;
    double mean_pixel_size_for_1_meter=0;
    double min_x, max_x, min_y, max_y;
    double x, y;
    QString utm_zone;
    bool first=true;

    // Compute and set UTM Zone using all images
    foreach (ProjectiveCamera* cam, m_camera_nodes) {
        mean_lat += cam->image()->navInfo().latitude();
        mean_lon += cam->image()->navInfo().longitude();
    }

    mean_lat /= (double)m_camera_nodes.size();
    mean_lon /= (double)m_camera_nodes.size();

    if ( !(m_T.latLongToUTM(mean_lat, mean_lon, x, y, utm_zone, false)) ){
        qDebug() << "Cannot retrieve UTM Zone\n";
        exit(1);
    }else{
        QStringList utm_params = utm_zone.split(" ");
        setUtmZone(utm_params.at(0).toInt());
        setUtmHemisphere(utm_params.at(1));
    }

    // Projection of all images navigation to utmZone of the MosaicDescriptor
    // Compute at the same time min and max for X & Y and meanAlt
    foreach (ProjectiveCamera* cam, m_camera_nodes) {

        if ( !(m_T.latLongToUTM(cam->image()->navInfo().latitude(),
                               cam->image()->navInfo().longitude(),
                               x, y, utm_zone, true)) ){
            qDebug() << "Cannot convert point to UTM\n";
            exit(1);
        }else{

            // Affect UTM values
            cam->image()->navInfo().setUtmX(x);
            cam->image()->navInfo().setUtmY(y);
            cam->image()->navInfo().setUtmZone(utm_zone);

            // Compute min max mean
            if (first){
                min_x = x;
                max_x = min_x;
                min_y = y;
                max_y = min_y;
                mean_alt = cam->image()->navInfo().altitude();
                mean_pixel_size_for_1_meter = 2/(cam->K().at<double>(0,0)+cam->K().at<double>(1,1));

                first = false;
            }else{
                if (x < min_x) min_x = x;
                if (x > max_x) max_x = x;
                if (y < min_y) min_y = y;
                if (y > max_y) max_y = y;
                mean_alt = mean_alt + cam->image()->navInfo().altitude();
                mean_pixel_size_for_1_meter += 2/(cam->K().at<double>(0,0)+cam->K().at<double>(1,1));
            }

        }
    }
    mean_alt /= (double)m_camera_nodes.size();
    mean_pixel_size_for_1_meter /= (double)m_camera_nodes.size();

    this->setMosaicOrigin(Point3d(min_x,max_y,mean_alt));
    this->setPixelSize(Point2d(mean_pixel_size_for_1_meter*mean_alt,mean_pixel_size_for_1_meter*mean_alt));

    // Compute the Transformation 3D Mosaic Frame -> 3D World Frame: W_X = W_R_M * M_X + W_T_M
    // Mosaic Rotation w.r.t. 3D World Frame
    m_W_R_M = m_T.rotX(CV_PI);
    // Mosaic Translation w.r.t. 3D World Frame
    // (The mosaic is translated according to the X, Y origin but not in moved in Z).
    m_W_T_M = (cv::Mat_<double>(3,1) << m_mosaic_origin.x, m_mosaic_origin.y, 0 );

    // Compute Inverse Transformation 3D World Frame -> 3D Mosaic Frame: M_X = M_R_W * W_X + M_T_W
    cv::transpose(m_W_R_M, m_M_R_W);
    m_M_T_W = -m_M_R_W * m_W_T_M;

    m_is_initialized = true;

    foreach (ProjectiveCamera* cam, m_camera_nodes) {
        computeCameraHomography(cam);
    }

}

void MosaicDescriptor::computeCameraHomography(ProjectiveCamera *_camera_p)
{
    if(!isInitialized()){
        qDebug() << "Initialize the MosaicDescriptor before calling this function. Exiting... \n";
        exit(1);
    }

    NavInfo * navdata = &(_camera_p->image()->navInfo());

    // The Yaw angle is Clockwise
    double yaw = -navdata->yaw();



    // Vehicle Rotation w.r.t. 3D World Frame (Computed always in mobile axis: post-multiplication)
    //  Rotation in Z (pi/2): Point X axis of the vehicle to the north to use the Yaw.
    //  Rotation in Z (Yaw): Yaw of the vehicle in the world frame.
    //  Rotation in X (Pi): Point Z axis of the vehicle looking down.
    //  Rotation in Y (Pitch): Pitch in the vehicle frame.
    //  Rotation in X (Roll): Roll in the vehicle frame.
    m_W_R_V = m_T.rotZ ( CV_PI / 2 ) * m_T.rotZ ( yaw ) * m_T.rotX ( CV_PI ) * m_T.rotY ( navdata->pitch() ) * m_T.rotX ( navdata->roll() );
    // Vehicle Translation w.r.t. 3D World Frame
    m_W_T_V = (cv::Mat_<double>(3,1) << navdata->utmX(), navdata->utmY(), navdata->altitude() );

    // Convert a Pose (V_T_C, V_R_C) w.r.t. 3D Vehicle Frame to the 3D World Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D World Frame: W_X = W_R_C * C_X + W_T_C
    m_W_R_C = m_W_R_V * _camera_p->V_R_C();
    m_W_T_C = m_W_R_V * _camera_p->V_T_C() + m_W_T_V;


    // Convert a Pose (W_T_C, W_R_C) w.r.t. 3D Camera Frame to the 3D Mosaic Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D Mosaic Frame: M_X = M_R_C * C_X + M_T_C
    m_M_R_C = m_M_R_W * m_W_R_C;
    m_M_T_C = m_M_R_W * m_W_T_C + m_M_T_W;

    // Compute Inverse Transformation 3D Mosaic Frame -> 3D Camera Frame: C_X = C_R_M * M_X + C_T_M
    cv::transpose(m_M_R_C,m_C_R_M);
    m_C_T_M = -m_C_R_M * m_M_T_C;

    // Compute Extrinsics using the 3D Mosaic Frame
    cv::hconcat(m_C_R_M, m_C_T_M, m_C_M_M);
    // Compute Projection Matrix (Intrinsics * Extrinsics): 3D Mosaic Frame -> 2D Image Plane
    m_i_P_M = _camera_p->K() * m_C_M_M;

    // Set Z = 0 (Delete 3th Column) in the Projection Matrix: 3D world to 2D
    // Up-To-Scale Planar Projective Homography: 2D Mosaic Frame to 2D Image Frame
    //_i_H_m = _i_P_M(:, [1 2 4]);
    //std::cerr << "_i_P_M = " << _i_P_M << std::endl;
    cv::hconcat(m_i_P_M.colRange(0,2),m_i_P_M.colRange(3,4), m_i_H_m);

    // Absolute Homography to be stored into the mosaic: 2D Image Plane to 2D Mosaic Frame
    // Add the scaling factor to have it in pixels.
    //std::cerr << "_i_H_m = " << _i_H_m << std::endl;
    cv::invert( m_i_H_m, m_m_H_i);

    _camera_p->setM_H_i_metric( m_m_H_i/ m_m_H_i.at<double>(2,2) );

    // Add the scaling factor to have it in pixels.
    m_m_H_i = m_hs * m_m_H_i;

    // Store Normalized Homography into the mosaic structure ([3,3] element is one).
    _camera_p->set_m_H_i( m_m_H_i / m_m_H_i.at<double>(2,2) );
    //std::cout << "_m_H_i 1 = " << _m_H_i << std::endl;
    //std::cout << "_m_H_i norm = " << _m_H_i / _m_H_i.at<double>(2,2) << std::endl;

}

void MosaicDescriptor::computeMosaicExtentAndShiftFrames()
{

    cv::Mat mosaic_bounds = (cv::Mat_<double>(2,2) << FLT_MAX, FLT_MAX, 0, 0 );
    int w, h = 0;

    cv::Mat pt1, pt2, pt3, pt4;
    std::vector<double> x_array, y_array;
    std::vector<double>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    foreach (ProjectiveCamera* cam, m_camera_nodes) {

        w = cam->image()->width();
        h = cam->image()->height();

        // Project corners_p on mosaic plane
        cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << 0,   0,   1), pt1);
        cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << w-1, 0,   1), pt2);
        cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << w-1, h-1, 1), pt3);
        cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << 0,   h-1, 1), pt4);

        // Fill x & y array
        x_array.clear();
        y_array.clear();
        x_array.push_back(pt1.at<double>(0,0)/pt1.at<double>(2,0));
        y_array.push_back(pt1.at<double>(1,0)/pt1.at<double>(2,0));
        x_array.push_back(pt2.at<double>(0,0)/pt2.at<double>(2,0));
        y_array.push_back(pt2.at<double>(1,0)/pt2.at<double>(2,0));
        x_array.push_back(pt3.at<double>(0,0)/pt3.at<double>(2,0));
        y_array.push_back(pt3.at<double>(1,0)/pt3.at<double>(2,0));
        x_array.push_back(pt4.at<double>(0,0)/pt4.at<double>(2,0));
        y_array.push_back(pt4.at<double>(1,0)/pt4.at<double>(2,0));


        // Compute min,max
        min_x_it = std::min_element(x_array.begin(), x_array.end());
        min_y_it = std::min_element(y_array.begin(), y_array.end());
        max_x_it = std::max_element(x_array.begin(), x_array.end());
        max_y_it = std::max_element(y_array.begin(), y_array.end());

        mosaic_bounds.at<double>(0,0) = std::min(*min_x_it, mosaic_bounds.at<double>(0, 0));
        mosaic_bounds.at<double>(1,0) = std::max(*max_x_it,mosaic_bounds.at<double>(1, 0));
        mosaic_bounds.at<double>(0,1) = std::min(*min_y_it,mosaic_bounds.at<double>(0, 1));
        mosaic_bounds.at<double>(1,1) = std::max(*max_y_it,mosaic_bounds.at<double>(1, 1));

    }

    // Shift all homographies with H
    cv::Mat H = (cv::Mat_<double>(3,3) << 1, 0, -mosaic_bounds.at<double>(0,0),
                 0, 1, -mosaic_bounds.at<double>(0,1),
                 0, 0, 1);

    foreach (ProjectiveCamera* cam, m_camera_nodes) {
        cam->set_m_H_i(H*cam->m_H_i());
    }

    // Shift origin
    m_mosaic_origin.x = m_mosaic_origin.x - (-mosaic_bounds.at<double>(0,0))*m_pixelm_size.x;
    m_mosaic_origin.y = m_mosaic_origin.y + (-mosaic_bounds.at<double>(0,1))*m_pixelm_size.y;
    m_mosaic_size.x = std::ceil(mosaic_bounds.at<double>(1,0)-mosaic_bounds.at<double>(0,0))+2; //+2 due to the 0 and the round
    m_mosaic_size.y = std::ceil(mosaic_bounds.at<double>(1,1)-mosaic_bounds.at<double>(0,1))+2;


    // Upper Left and Lower Right corner coordinates
    double x_shift = (mosaic_bounds.at<double>(1,0)-mosaic_bounds.at<double>(0,0));
    double y_shift = (mosaic_bounds.at<double>(1,1)-mosaic_bounds.at<double>(0,1));
    m_mosaic_ullr = (cv::Mat_<double>(4,1) << m_mosaic_origin.x, m_mosaic_origin.y,
                    m_mosaic_origin.x+x_shift*m_pixelm_size.x, m_mosaic_origin.y-y_shift*m_pixelm_size.y);


}

void MosaicDescriptor::writeToGeoTiff(Mat &raster_p, Mat &rasterMask_p, QString filePath_p)
{

    QString utm_proj_param, utm_hemisphere_option, utm_zone_string;

    // Construct utm proj param options
    utm_zone_string = QString("%1 ").arg(utmZone())+ utmHemisphere();
    QStringList utm_params = utm_zone_string.split(" ");

    if ( utm_params.at(1) == "S" ){
        utm_hemisphere_option = QString(" +south");
    }else{
        utm_hemisphere_option = QString("");
    }
    utm_proj_param = QString("+proj=utm +zone=") + utm_params.at(0);

    QString gdal_options =  QString("-a_srs \"")+ utm_proj_param + QString("\" -of GTiff -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
            .arg(mosaic_ullr().at<double>(0,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(1,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(2,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(3,0),0,'f',2);
    RasterGeoreferencer raster_georef;
    raster_georef.writeGeoFile(raster_p,rasterMask_p,filePath_p,gdal_options);

}


QVector<ProjectiveCamera *> MosaicDescriptor::cameraNodes() const
{
    return m_camera_nodes;
}

bool MosaicDescriptor::isInitialized() const
{
    return m_is_initialized;
}

void MosaicDescriptor::decimateImagesFromOverlap(double _min_overlap_p, double _max_overlap_p)
{


    // Allocate and build polygons associated with images
    std::vector<Polygon*> vp_images_poly;


    for (int k=0; k < cameraNodes().size(); k++){
        std::vector<double> x, y;

        // Construct currentPolygon
        cameraNodes().at(k)->computeImageFootPrint(x,y);
        Polygon *current_polygon = new Polygon();
        //x.push_back(xBegin); x.push_back(xEnd); x.push_back(xEnd); x.push_back(xBegin);
        //y.push_back(yBegin); y.push_back(yBegin); y.push_back(yEnd); y.push_back(yEnd);
        current_polygon->addContour(x,y);
        x.clear(); y.clear();
        vp_images_poly.push_back(current_polygon);

    }

    // Decimate images
    int i_curr = 0;
    std::vector<bool> kept_indexes;
    kept_indexes.push_back(true);

    for (int i_next=1; i_next<(cameraNodes().size()-1 ); i_next++){

        double area_ratio_1 = vp_images_poly.at(i_curr)->clipArea(*vp_images_poly.at(i_next), basic_processing::INT)/vp_images_poly.at(i_curr)->area();
        double area_ratio_2 = vp_images_poly.at(i_next)->clipArea(*vp_images_poly.at(i_next+1), basic_processing::INT)/vp_images_poly.at(i_next)->area();

        if (area_ratio_1<_max_overlap_p || area_ratio_2<_min_overlap_p){
            kept_indexes.push_back(true);
            i_curr = i_next;
        }else{
            kept_indexes.push_back(false);
        }

    }
    // include last image
    kept_indexes.push_back(true);

    // Recreate cameraNode and delete non needed cameras
    QVector<ProjectiveCamera*> temp_camera_nodes;
    for (unsigned int i=0; i<kept_indexes.size(); i++){
        if (kept_indexes.at(i)==true){
            temp_camera_nodes.push_back(m_camera_nodes.at(i));
        }else{
            delete m_camera_nodes.at(i);
        }
    }

    m_camera_nodes = temp_camera_nodes;


    // Delete Polygons from memory
    for (unsigned int i=0; i<vp_images_poly.size(); i++){
        delete vp_images_poly.at(i);
    }


}

void MosaicDescriptor::decimateImagesUntilNoOverlap()
{
    // Allocate and build polygons associated with images
    std::vector<Polygon*> vp_images_poly;

    double eps = 0.000001;

    Polygon* poly_union = new Polygon();
    Polygon* temp_union = new Polygon();

    for (int k = 0; k < cameraNodes().size(); k++) {
        std::vector<double> x, y;

        // Construct currentPolygon
        cameraNodes().at(k)->computeImageFootPrint(x, y);
        Polygon* current_polygon = new Polygon();

        current_polygon->addContour(x, y);
        x.clear(); y.clear();
        vp_images_poly.push_back(current_polygon);

    }

    // Decimate images
    std::vector<bool> kept_indexes;
    kept_indexes.push_back(true);

    // Initialize union
    *poly_union = *(vp_images_poly.at(0));

    for (int i = 1; i < cameraNodes().size(); i++) {

        double inter_area = poly_union->clipArea(*vp_images_poly.at(i), basic_processing::INT);

        if (inter_area <eps) {
            kept_indexes.push_back(true);
            poly_union->clip(*vp_images_poly.at(i), *temp_union, basic_processing::UNION);
            *poly_union = *temp_union;
        }
        else {
            kept_indexes.push_back(false);
        }

    }

    // Recreate cameraNode and delete non needed cameras
    QVector<ProjectiveCamera*> temp_camera_nodes;
    for (unsigned int i = 0; i < kept_indexes.size(); i++) {
        if (kept_indexes.at(i) == true) {
            temp_camera_nodes.push_back(m_camera_nodes.at(i));
        }
        else {
            delete m_camera_nodes.at(i);
        }
    }

    m_camera_nodes = temp_camera_nodes;


    // Delete Polygons from memory
    for (unsigned int i = 0; i < vp_images_poly.size(); i++) {
        delete vp_images_poly.at(i);
    }
    delete poly_union;
    delete temp_union;
}

} // namespace optical_mapping
