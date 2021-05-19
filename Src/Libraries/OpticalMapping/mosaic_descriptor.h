#ifndef MOSAIC_H
#define MOSAIC_H


#include "nav_image.h"
#include "projective_camera.h"
#include "geo_transform.h"
#include <QVector>
#include <QString>
#include <QMetaType>

using namespace MatisseCommon;
//using namespace cv;


// Positionning and scaling parameters
struct MosaicSizesAndExtent {
    cv::Point3d _mosaicOrigin;
    cv::Point2d _pixelSize;
    cv::Point2d _mosaicSize;
    cv::Mat _mosaic_ullr;
    QString _utmHemisphere;
    int _utmZone;
};


class MosaicDescriptor
{

public:
    MosaicDescriptor();
    ~MosaicDescriptor();

    // ******** Attributes setters & getters ********

    cv::Point3d mosaicOrigin() const;
    void setMosaicOrigin(const cv::Point3d &mosaicOrigin);

    cv::Point2d pixelSize() const;
    void setPixelSize(const cv::Point2d &pixelSize);

    cv::Point2d mosaicSize() const;
    void setMosaicSize(const cv::Point2d &mosaicSize);

    QString utmHemisphere() const;
    void setUtmHemisphere(const QString &utmHemisphere);

    int utmZone() const;
    void setUtmZone(int utmZone);

    cv::Mat mosaic_ullr() const;
    void setMosaic_ullr(const cv::Mat &mosaic_ullr);

    QVector<ProjectiveCamera *> cameraNodes() const;

    bool isInitialized() const;

    // **********************************************

    ///
    /// \brief initCamerasAndFrames initialize MosaicDescriptor from the whole Camera's dataset
    ///  Only use in case of post-treatment with the complete dataset.
    /// \param cameraNodes_p whole dataset Cameras
    /// \param camerasOwner true if ProjectiveCamera* belongs to this class (hence are deleted by this class)
    ///
    void initCamerasAndFrames(QVector<ProjectiveCamera*> cameraNodes_p, bool camerasOwner_p = true);

    ///
    /// \brief computeCameraHomography compute camera homography from nav and MosaicDescriptor init
    /// \warning The mosaicDescriptor class has to be initialized before calling this function
    /// \param camera_p
    ///
    void computeCameraHomography(ProjectiveCamera* camera_p);

    ///
    /// \brief computeMosaicExtentAndShiftFrames This function compute the mosaic extent,
    /// recompute mosaic origin such that no negative pixels coord fall into the mosaicking
    ///
    void computeMosaicExtentAndShiftFrames();

    ///
    /// \brief writeToGeoTiff writes a geotiff file from its description
    /// \param raster_p raster image obtained from mosaicDrawer
    /// \param rasterMask_p raster mask image obtained from mosaicDrawer
    /// \param filePath_p output file path
    ///
    void writeToGeoTiff(cv::Mat &raster_p, cv::Mat &rasterMask_p, QString filePath_p);

    ///
    /// \brief decimateImagesFromOverlap This function removes images that have to much overlap with each others
    /// \param minOverlap_p minimum required overlap
    /// \param maxOverlap_p maximum overlap before decimation
    ///
    void decimateImagesFromOverlap(double minOverlap_p=0.5, double maxOverlap_p=0.7);

    ///
    /// \brief decimateImagesUntilNoOverlap This function removes images that have overlap with past images (This help to not count twice structures in case of bad navigation)
    ///
    void decimateImagesUntilNoOverlap();


protected:

    // Positionning and scaling parameters
    cv::Point3d _mosaicOrigin;
    cv::Point2d _pixelSize;
    cv::Point2d _mosaicSize;
    cv::Mat _mosaic_ullr;
    cv::Mat _Hs; // ScalingMatrix
    QString _utmHemisphere;
    int _utmZone;

    GeoTransform _T;

    bool _camerasOwner;
    bool _isInitialized;

    // Frames parameters
    cv::Mat _W_R_M,_W_T_M;
    cv::Mat _M_R_W,_M_T_W;
    cv::Mat _W_R_V,_W_T_V;
    cv::Mat _W_R_C,_W_T_C;
    cv::Mat _M_R_C,_M_T_C;
    cv::Mat _C_R_M,_C_T_M;
    cv::Mat _C_M_M;
    cv::Mat _i_P_M;
    cv::Mat _i_H_m, _m_H_i;

    // The set of cameras that took each iamge of the mosaic
    // Note : it can be the same camera moving of multiple cameras
    QVector<ProjectiveCamera*> _cameraNodes;

};

Q_DECLARE_METATYPE(QVector<ProjectiveCamera *> *)
Q_DECLARE_METATYPE(MosaicDescriptor *)

#endif // MOSAIC_H
