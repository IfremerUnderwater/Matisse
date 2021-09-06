#ifndef OPTICAL_MAPPING_MOSAIC_DESCRIPTOR_H_
#define OPTICAL_MAPPING_MOSAIC_DESCRIPTOR_H_


#include "nav_image.h"
#include "projective_camera.h"
#include "geo_transform.h"
#include <QVector>
#include <QString>
#include <QMetaType>


namespace optical_mapping {

// Positionning and scaling parameters
struct MosaicSizesAndExtent {
    cv::Point3d mosaic_origin;
    cv::Point2d pixel_size;
    cv::Point2d mosaic_size;
    cv::Mat mosaic_ullr;
    QString utm_hemisphere;
    int utm_zone;
};


class MosaicDescriptor
{

public:
    MosaicDescriptor();
    ~MosaicDescriptor();

    // ******** Attributes setters & getters ********

    cv::Point3d mosaicOrigin() const;
    void setMosaicOrigin(const cv::Point3d &_mosaic_origin);

    cv::Point2d pixelSize() const;
    void setPixelSize(const cv::Point2d &_pixel_size);

    cv::Point2d mosaicSize() const;
    void setMosaicSize(const cv::Point2d &_mosaic_size);

    QString utmHemisphere() const;
    void setUtmHemisphere(const QString &_utm_hemisphere);

    int utmZone() const;
    void setUtmZone(int _utm_zone);

    cv::Mat mosaic_ullr() const;
    void setMosaic_ullr(const cv::Mat &_mosaic_ullr);

    QVector<ProjectiveCamera *> cameraNodes() const;

    bool isInitialized() const;

    // **********************************************

    ///
    /// \brief initCamerasAndFrames initialize MosaicDescriptor from the whole Camera's dataset
    ///  Only use in case of post-treatment with the complete dataset.
    /// \param _camera_nodes_p whole dataset Cameras
    /// \param _cameras_owner true if ProjectiveCamera* belongs to this class (hence are deleted by this class)
    ///
    void initCamerasAndFrames(QVector<ProjectiveCamera*> _camera_nodes_p, bool _cameras_owner_p = true);

    ///
    /// \brief computeCameraHomography compute camera homography from nav and MosaicDescriptor init
    /// \warning The mosaicDescriptor class has to be initialized before calling this function
    /// \param _camera_p
    ///
    void computeCameraHomography(ProjectiveCamera* _camera_p);

    ///
    /// \brief computeMosaicExtentAndShiftFrames This function compute the mosaic extent,
    /// recompute mosaic origin such that no negative pixels coord fall into the mosaicking
    ///
    void computeMosaicExtentAndShiftFrames();

    ///
    /// \brief writeToGeoTiff writes a geotiff file from its description
    /// \param _raster_p raster image obtained from mosaicDrawer
    /// \param _raster_mask_p raster mask image obtained from mosaicDrawer
    /// \param _file_path_p output file path
    ///
    void writeToGeoTiff(cv::Mat &_raster_p, cv::Mat &_raster_mask_p, QString _file_path_p);

    ///
    /// \brief decimateImagesFromOverlap This function removes images that have to much overlap with each others
    /// \param _min_overlap_p minimum required overlap
    /// \param _max_overlap_p maximum overlap before decimation
    ///
    void decimateImagesFromOverlap(double _min_overlap_p=0.5, double _max_overlap_p=0.7);

    ///
    /// \brief decimateImagesUntilNoOverlap This function removes images that have overlap with past images (This help to not count twice structures in case of bad navigation)
    ///
    void decimateImagesUntilNoOverlap();


protected:

    // Positionning and scaling parameters
    cv::Point3d m_mosaic_origin;
    cv::Point2d m_pixelm_size;
    cv::Point2d m_mosaic_size;
    cv::Mat m_mosaic_ullr;
    cv::Mat m_hs; // ScalingMatrix
    QString m_utm_hemisphere;
    int m_utm_zone;

    nav_tools::GeoTransform m_T;

    bool m_cameras_owner;
    bool m_is_initialized;

    // Frames parameters
    cv::Mat m_W_R_M, m_W_T_M;
    cv::Mat m_M_R_W, m_M_T_W;
    cv::Mat m_W_R_V, m_W_T_V;
    cv::Mat m_W_R_C, m_W_T_C;
    cv::Mat m_M_R_C, m_M_T_C;
    cv::Mat m_C_R_M, m_C_T_M;
    cv::Mat m_C_M_M;
    cv::Mat m_i_P_M;
    cv::Mat m_i_H_m, m_m_H_i;

    // The set of cameras that took each iamge of the mosaic
    // Note : it can be the same camera moving of multiple cameras
    QVector<ProjectiveCamera*> m_camera_nodes;

};

} // namespace optical_mapping

Q_DECLARE_METATYPE(QVector<optical_mapping::ProjectiveCamera *> *)
Q_DECLARE_METATYPE(optical_mapping::MosaicDescriptor *)

#endif // OPTICAL_MAPPING_MOSAIC_DESCRIPTOR_H_
