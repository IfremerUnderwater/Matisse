#include "mosaic_descriptor.h"
#include <QDebug>
#include <float.h>
#include <algorithm>
#include <vector>
#include <iterator>
#include "RasterGeoreferencer.h"
#include "Polygon.h"

using namespace basicproc;
using namespace cv;

MosaicDescriptor::MosaicDescriptor():_mosaicOrigin(0,0,0),
    _pixelSize(0,0),_mosaicSize(0,0), _utmHemisphere("UNDEF"),
    _utmZone(-1),_camerasOwner(true), _isInitialized(false)
{

    _mosaic_ullr.zeros(1,4,CV_64F);

}

MosaicDescriptor::~MosaicDescriptor()
{
    if(_camerasOwner){

        if (!_cameraNodes.isEmpty()){
            foreach (ProjectiveCamera* Cam, _cameraNodes) {
                delete Cam;
            }
        }
    }
}

Point3d MosaicDescriptor::mosaicOrigin() const
{
    return _mosaicOrigin;
}

void MosaicDescriptor::setMosaicOrigin(const Point3d &mosaicOrigin)
{
    _mosaicOrigin = mosaicOrigin;
}
Point2d MosaicDescriptor::pixelSize() const
{
    return _pixelSize;
}

void MosaicDescriptor::setPixelSize(const Point2d &pixelSize)
{
    _pixelSize = pixelSize;

    _Hs = (cv::Mat_<double>(3,3) <<
           1.0/_pixelSize.x,               0,       0,
           0,               1.0/_pixelSize.y,       0,
           0,                              0,       1);
}
Point2d MosaicDescriptor::mosaicSize() const
{
    return _mosaicSize;
}

void MosaicDescriptor::setMosaicSize(const Point2d &mosaicSize)
{
    _mosaicSize = mosaicSize;
}
QString MosaicDescriptor::utmHemisphere() const
{
    return _utmHemisphere;
}

void MosaicDescriptor::setUtmHemisphere(const QString &utmHemisphere)
{
    _utmHemisphere = utmHemisphere;
}
int MosaicDescriptor::utmZone() const
{
    return _utmZone;
}

void MosaicDescriptor::setUtmZone(int utmZone)
{
    _utmZone = utmZone;
}
Mat MosaicDescriptor::mosaic_ullr() const
{
    return _mosaic_ullr;
}

void MosaicDescriptor::setMosaic_ullr(const Mat &mosaic_ullr)
{
    _mosaic_ullr = mosaic_ullr;
}

void MosaicDescriptor::initCamerasAndFrames(QVector<ProjectiveCamera*> cameras_p, bool camerasOwner_p)
{
    _camerasOwner = camerasOwner_p;

    _cameraNodes = cameras_p;
    double meanLat=0;
    double meanLon=0;
    double meanAlt=0;
    double meanPixelSizeFor1meter=0;
    double minX, maxX, minY, maxY;
    double X,Y;
    QString utmZone;
    bool first=true;

    // Compute and set UTM Zone using all images
    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        meanLat += Cam->image()->navInfo().latitude();
        meanLon += Cam->image()->navInfo().longitude();
    }

    meanLat /= (double)_cameraNodes.size();
    meanLon /= (double)_cameraNodes.size();

    if ( !(_T.LatLongToUTM(meanLat, meanLon, X, Y, utmZone, false)) ){
        qDebug() << "Cannot retrieve UTM Zone\n";
        exit(1);
    }else{
        QStringList utmParams = utmZone.split(" ");
        setUtmZone(utmParams.at(0).toInt());
        setUtmHemisphere(utmParams.at(1));
    }

    // Projection of all images navigation to utmZone of the MosaicDescriptor
    // Compute at the same time min and max for X & Y and meanAlt
    foreach (ProjectiveCamera* Cam, _cameraNodes) {

        if ( !(_T.LatLongToUTM(Cam->image()->navInfo().latitude(),
                               Cam->image()->navInfo().longitude(),
                               X, Y, utmZone, true)) ){
            qDebug() << "Cannot convert point to UTM\n";
            exit(1);
        }else{

            // Affect UTM values
            Cam->image()->navInfo().setUtmX(X);
            Cam->image()->navInfo().setUtmY(Y);
            Cam->image()->navInfo().setUtmZone(utmZone);

            // Compute min max mean
            if (first){
                minX = X;
                maxX = minX;
                minY = Y;
                maxY = minY;
                meanAlt = Cam->image()->navInfo().altitude();
                meanPixelSizeFor1meter = 2/(Cam->K().at<double>(0,0)+Cam->K().at<double>(1,1));

                first = false;
            }else{
                if (X < minX) minX = X;
                if (X > maxX) maxX = X;
                if (Y < minY) minY = Y;
                if (Y > maxY) maxY = Y;
                meanAlt = meanAlt + Cam->image()->navInfo().altitude();
                meanPixelSizeFor1meter += 2/(Cam->K().at<double>(0,0)+Cam->K().at<double>(1,1));
            }

        }
    }
    meanAlt /= (double)_cameraNodes.size();
    meanPixelSizeFor1meter /= (double)_cameraNodes.size();

    this->setMosaicOrigin(Point3d(minX,maxY,meanAlt));
    this->setPixelSize(Point2d(meanPixelSizeFor1meter*meanAlt,meanPixelSizeFor1meter*meanAlt));

    // Compute the Transformation 3D Mosaic Frame -> 3D World Frame: W_X = W_R_M * M_X + W_T_M
    // Mosaic Rotation w.r.t. 3D World Frame
    _W_R_M = _T.RotX(CV_PI);
    // Mosaic Translation w.r.t. 3D World Frame
    // (The mosaic is translated according to the X, Y origin but not in moved in Z).
    _W_T_M = (cv::Mat_<double>(3,1) << _mosaicOrigin.x, _mosaicOrigin.y, 0 );

    // Compute Inverse Transformation 3D World Frame -> 3D Mosaic Frame: M_X = M_R_W * W_X + M_T_W
    cv::transpose(_W_R_M, _M_R_W);
    _M_T_W = -_M_R_W * _W_T_M;

    _isInitialized = true;

    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        computeCameraHomography(Cam);
    }

}

void MosaicDescriptor::computeCameraHomography(ProjectiveCamera *camera_p)
{
    if(!isInitialized()){
        qDebug() << "Initialize the MosaicDescriptor before calling this function. Exiting... \n";
        exit(1);
    }

    NavInfo * navdata = &(camera_p->image()->navInfo());

    // The Yaw angle is Clockwise
    double Yaw = -navdata->yaw();



    // Vehicle Rotation w.r.t. 3D World Frame (Computed always in mobile axis: post-multiplication)
    //  Rotation in Z (pi/2): Point X axis of the vehicle to the north to use the Yaw.
    //  Rotation in Z (Yaw): Yaw of the vehicle in the world frame.
    //  Rotation in X (Pi): Point Z axis of the vehicle looking down.
    //  Rotation in Y (Pitch): Pitch in the vehicle frame.
    //  Rotation in X (Roll): Roll in the vehicle frame.
    _W_R_V = _T.RotZ ( CV_PI / 2 ) * _T.RotZ ( Yaw ) * _T.RotX ( CV_PI ) * _T.RotY ( navdata->pitch() ) * _T.RotX ( navdata->roll() );
    // Vehicle Translation w.r.t. 3D World Frame
    _W_T_V = (cv::Mat_<double>(3,1) << navdata->utmX(), navdata->utmY(), navdata->altitude() );

    // Convert a Pose (V_T_C, V_R_C) w.r.t. 3D Vehicle Frame to the 3D World Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D World Frame: W_X = W_R_C * C_X + W_T_C
    _W_R_C = _W_R_V * camera_p->V_R_C();
    _W_T_C = _W_R_V * camera_p->V_T_C() + _W_T_V;


    // Convert a Pose (W_T_C, W_R_C) w.r.t. 3D Camera Frame to the 3D Mosaic Frame
    // This new pose will also be the Transformation 3D Camera Frame -> 3D Mosaic Frame: M_X = M_R_C * C_X + M_T_C
    _M_R_C = _M_R_W * _W_R_C;
    _M_T_C = _M_R_W * _W_T_C + _M_T_W;

    // Compute Inverse Transformation 3D Mosaic Frame -> 3D Camera Frame: C_X = C_R_M * M_X + C_T_M
    cv::transpose(_M_R_C,_C_R_M);
    _C_T_M = -_C_R_M * _M_T_C;

    // Compute Extrinsics using the 3D Mosaic Frame
    cv::hconcat(_C_R_M, _C_T_M, _C_M_M);
    // Compute Projection Matrix (Intrinsics * Extrinsics): 3D Mosaic Frame -> 2D Image Plane
    _i_P_M = camera_p->K() * _C_M_M;

    // Set Z = 0 (Delete 3th Column) in the Projection Matrix: 3D world to 2D
    // Up-To-Scale Planar Projective Homography: 2D Mosaic Frame to 2D Image Frame
    //_i_H_m = _i_P_M(:, [1 2 4]);
    //std::cerr << "_i_P_M = " << _i_P_M << std::endl;
    cv::hconcat(_i_P_M.colRange(0,2),_i_P_M.colRange(3,4), _i_H_m);

    // Absolute Homography to be stored into the mosaic: 2D Image Plane to 2D Mosaic Frame
    // Add the scaling factor to have it in pixels.
    //std::cerr << "_i_H_m = " << _i_H_m << std::endl;
    cv::invert( _i_H_m, _m_H_i);

    camera_p->setM_H_i_metric( _m_H_i/ _m_H_i.at<double>(2,2) );

    // Add the scaling factor to have it in pixels.
    _m_H_i = _Hs * _m_H_i;

    // Store Normalized Homography into the mosaic structure ([3,3] element is one).
    camera_p->set_m_H_i( _m_H_i / _m_H_i.at<double>(2,2) );
    //std::cout << "_m_H_i 1 = " << _m_H_i << std::endl;
    //std::cout << "_m_H_i norm = " << _m_H_i / _m_H_i.at<double>(2,2) << std::endl;

}

void MosaicDescriptor::computeMosaicExtentAndShiftFrames()
{

    cv::Mat mosaicbounds = (cv::Mat_<double>(2,2) << FLT_MAX, FLT_MAX, 0, 0 );
    int w,h =0;

    cv::Mat pt1,pt2,pt3,pt4;
    std::vector<double> xArray, yArray;
    std::vector<double>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    foreach (ProjectiveCamera* Cam, _cameraNodes) {

        w = Cam->image()->width();
        h = Cam->image()->height();

        // Project corners_p on mosaic plane
        Cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << 0,   0,   1), pt1);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << w-1, 0,   1), pt2);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << w-1, h-1, 1), pt3);
        Cam->projectPtOnMosaickingPlane((cv::Mat_<double>(3,1) << 0,   h-1, 1), pt4);

        // Fill x & y array
        xArray.clear();
        yArray.clear();
        xArray.push_back(pt1.at<double>(0,0)/pt1.at<double>(2,0));
        yArray.push_back(pt1.at<double>(1,0)/pt1.at<double>(2,0));
        xArray.push_back(pt2.at<double>(0,0)/pt2.at<double>(2,0));
        yArray.push_back(pt2.at<double>(1,0)/pt2.at<double>(2,0));
        xArray.push_back(pt3.at<double>(0,0)/pt3.at<double>(2,0));
        yArray.push_back(pt3.at<double>(1,0)/pt3.at<double>(2,0));
        xArray.push_back(pt4.at<double>(0,0)/pt4.at<double>(2,0));
        yArray.push_back(pt4.at<double>(1,0)/pt4.at<double>(2,0));


        // Compute min,max
        min_x_it = std::min_element(xArray.begin(), xArray.end());
        min_y_it = std::min_element(yArray.begin(), yArray.end());
        max_x_it = std::max_element(xArray.begin(), xArray.end());
        max_y_it = std::max_element(yArray.begin(), yArray.end());

        mosaicbounds.at<double>(0,0) = std::min(*min_x_it, mosaicbounds.at<double>(0, 0));
        mosaicbounds.at<double>(1,0) = std::max(*max_x_it,mosaicbounds.at<double>(1, 0));
        mosaicbounds.at<double>(0,1) = std::min(*min_y_it,mosaicbounds.at<double>(0, 1));
        mosaicbounds.at<double>(1,1) = std::max(*max_y_it,mosaicbounds.at<double>(1, 1));

    }

    // Shift all homographies with H
    cv::Mat H = (cv::Mat_<double>(3,3) << 1, 0, -mosaicbounds.at<double>(0,0),
                 0, 1, -mosaicbounds.at<double>(0,1),
                 0, 0, 1);

    foreach (ProjectiveCamera* Cam, _cameraNodes) {
        Cam->set_m_H_i(H*Cam->m_H_i());
    }

    // Shift origin
    _mosaicOrigin.x = _mosaicOrigin.x - (-mosaicbounds.at<double>(0,0))*_pixelSize.x;
    _mosaicOrigin.y = _mosaicOrigin.y + (-mosaicbounds.at<double>(0,1))*_pixelSize.y;
    _mosaicSize.x = std::ceil(mosaicbounds.at<double>(1,0)-mosaicbounds.at<double>(0,0))+2; //+2 due to the 0 and the round
    _mosaicSize.y = std::ceil(mosaicbounds.at<double>(1,1)-mosaicbounds.at<double>(0,1))+2;


    // Upper Left and Lower Right corner coordinates
    double x_shift = (mosaicbounds.at<double>(1,0)-mosaicbounds.at<double>(0,0));
    double y_shift = (mosaicbounds.at<double>(1,1)-mosaicbounds.at<double>(0,1));
    _mosaic_ullr = (cv::Mat_<double>(4,1) << _mosaicOrigin.x, _mosaicOrigin.y,
                    _mosaicOrigin.x+x_shift*_pixelSize.x, _mosaicOrigin.y-y_shift*_pixelSize.y);


}

void MosaicDescriptor::writeToGeoTiff(Mat &raster_p, Mat &rasterMask_p, QString filePath_p)
{

    QString utmProjParam, utmHemisphereOption,utmZoneString;

    // Construct utm proj param options
    utmZoneString = QString("%1 ").arg(utmZone())+ utmHemisphere();
    QStringList utmParams = utmZoneString.split(" ");

    if ( utmParams.at(1) == "S" ){
        utmHemisphereOption = QString(" +south");
    }else{
        utmHemisphereOption = QString("");
    }
    utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0);

    QString gdalOptions =  QString("-a_srs \"")+ utmProjParam + QString("\" -of GTiff -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
            .arg(mosaic_ullr().at<double>(0,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(1,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(2,0),0,'f',2)
            .arg(mosaic_ullr().at<double>(3,0),0,'f',2);
    RasterGeoreferencer rasterGeoref;
    rasterGeoref.WriteGeoFile(raster_p,rasterMask_p,filePath_p,gdalOptions);

}


QVector<ProjectiveCamera *> MosaicDescriptor::cameraNodes() const
{
    return _cameraNodes;
}

bool MosaicDescriptor::isInitialized() const
{
    return _isInitialized;
}

void MosaicDescriptor::decimateImagesFromOverlap(double minOverlap_p, double maxOverlap_p)
{


    // Allocate and build polygons associated with images
    std::vector<Polygon*> vpImagesPoly;


    for (int k=0; k < cameraNodes().size(); k++){
        std::vector<double> x,y;

        // Construct currentPolygon
        cameraNodes().at(k)->computeImageFootPrint(x,y);
        Polygon *currentPolygon = new Polygon();
        //x.push_back(xBegin); x.push_back(xEnd); x.push_back(xEnd); x.push_back(xBegin);
        //y.push_back(yBegin); y.push_back(yBegin); y.push_back(yEnd); y.push_back(yEnd);
        currentPolygon->addContour(x,y);
        x.clear(); y.clear();
        vpImagesPoly.push_back(currentPolygon);

    }

    // Decimate images
    int i_curr = 0;
    std::vector<bool> keptIndexes;
    keptIndexes.push_back(true);

    for (int i_next=1; i_next<(cameraNodes().size()-1 ); i_next++){

        double area_ratio_1 = vpImagesPoly.at(i_curr)->clipArea(*vpImagesPoly.at(i_next), basicproc::INT)/vpImagesPoly.at(i_curr)->area();
        double area_ratio_2 = vpImagesPoly.at(i_next)->clipArea(*vpImagesPoly.at(i_next+1), basicproc::INT)/vpImagesPoly.at(i_next)->area();

        if (area_ratio_1<maxOverlap_p || area_ratio_2<minOverlap_p){
            keptIndexes.push_back(true);
            i_curr = i_next;
        }else{
            keptIndexes.push_back(false);
        }

    }
    // include last image
    keptIndexes.push_back(true);

    // Recreate cameraNode and delete non needed cameras
    QVector<ProjectiveCamera*> tempCameraNodes;
    for (unsigned int i=0; i<keptIndexes.size(); i++){
        if (keptIndexes.at(i)==true){
            tempCameraNodes.push_back(_cameraNodes.at(i));
        }else{
            delete _cameraNodes.at(i);
        }
    }

    _cameraNodes = tempCameraNodes;


    // Delete Polygons from memory
    for (unsigned int i=0; i<vpImagesPoly.size(); i++){
        delete vpImagesPoly.at(i);
    }


}

void MosaicDescriptor::decimateImagesUntilNoOverlap()
{
    // Allocate and build polygons associated with images
    std::vector<Polygon*> vpImagesPoly;

    double eps = 0.000001;

    Polygon* poly_union = new Polygon();
    Polygon* temp_union = new Polygon();

    for (int k = 0; k < cameraNodes().size(); k++) {
        std::vector<double> x, y;

        // Construct currentPolygon
        cameraNodes().at(k)->computeImageFootPrint(x, y);
        Polygon* currentPolygon = new Polygon();

        currentPolygon->addContour(x, y);
        x.clear(); y.clear();
        vpImagesPoly.push_back(currentPolygon);

    }

    // Decimate images
    std::vector<bool> keptIndexes;
    keptIndexes.push_back(true);

    // Initialize union
    *poly_union = *(vpImagesPoly.at(0));

    for (int i = 1; i < cameraNodes().size(); i++) {

        double inter_area = poly_union->clipArea(*vpImagesPoly.at(i), basicproc::INT);

        if (inter_area <eps) {
            keptIndexes.push_back(true);
            poly_union->clip(*vpImagesPoly.at(i), *temp_union, basicproc::UNION);
            *poly_union = *temp_union;
        }
        else {
            keptIndexes.push_back(false);
        }

    }

    // Recreate cameraNode and delete non needed cameras
    QVector<ProjectiveCamera*> tempCameraNodes;
    for (unsigned int i = 0; i < keptIndexes.size(); i++) {
        if (keptIndexes.at(i) == true) {
            tempCameraNodes.push_back(_cameraNodes.at(i));
        }
        else {
            delete _cameraNodes.at(i);
        }
    }

    _cameraNodes = tempCameraNodes;


    // Delete Polygons from memory
    for (unsigned int i = 0; i < vpImagesPoly.size(); i++) {
        delete vpImagesPoly.at(i);
    }
    delete poly_union;
    delete temp_union;
}
