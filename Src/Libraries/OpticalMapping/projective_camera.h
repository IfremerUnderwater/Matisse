#ifndef OPTICAL_MAPPING_PROJECTIVE_CAMERA_H_
#define OPTICAL_MAPPING_PROJECTIVE_CAMERA_H_



#include <opencv2/opencv.hpp>
#include <nav_image.h>

using namespace matisse_image;

namespace optical_mapping {

class ProjectiveCamera
{
public:
    ProjectiveCamera();
    ProjectiveCamera(NavImage *_image_p, cv::Mat _camera_matrix_K_p, cv::Mat _V_T_C, cv::Mat _V_R_C, double _scale_factor_p);

    // **** Attributes setters & getters ****

    cv::Mat K() const;
    void setK(const cv::Mat &_K);

    cv::Mat m_H_i() const;
    void set_m_H_i(const cv::Mat &_m_H_i);

    cv::Mat m_H_i_metric() const;
    void setM_H_i_metric(const cv::Mat &_m_H_i_metric);

    cv::Mat V_T_C() const;
    void setV_T_C(const cv::Mat &_V_T_C);

    cv::Mat V_R_C() const;
    void setV_R_C(const cv::Mat &_V_R_C);

    NavImage *image() const;
    void setImage(NavImage *_image);

    double scalingFactor() const;
    void setScalingFactor(const double &_scaling_factor);

    // **************************************

    ///
    /// \brief projectPtOnMosaickingPlane project a camera plane point to mosaic plane point
    /// \param _cam_plane_pt_p camera plane point
    /// \param _mosaic_plane_pt_p mosaic plane point
    /// \param _metric use metric frame if set to true
    ///
    void projectPtOnMosaickingPlane(const cv::Mat _cam_plane_pt_p, cv::Mat & _mosaic_plane_pt_p, bool _metric_p=false);

    ///
    /// \brief projectImageOnMosaickingPlane project camera plane image to mosaic plane point
    /// \param _cam_plane_pt_p camera plane point
    /// \param _mosaic_plane_pt_p mosaic plane point
    /// \param _corner_p
    ///
    void projectImageOnMosaickingPlane(cv::UMat & _mosaic_plane_image_p, cv::UMat & mosaic_plane_mask_p, cv::Point & _corner_p);

    ///
    /// \brief computeImageExtent compute image origin and size
    /// \param _corner_p origin of the image in the mosaicking frame
    /// \param _dst_size size of the image projected on the mosaicking plane
    ///
    void computeImageExtent(cv::Point &_corner_p, cv::Size &_dst_size_p);

    ///
    /// \brief computeImageFootPrint compute image footprint coords on the mosaicking plane
    /// \param _x_array x coords
    /// \param _y_array y coords
    ///
    void computeImageFootPrint(std::vector<double> &_x_array, std::vector<double> &_y_array);


private:

    cv::Mat m_K; // Camera calibration matrix (intrinsics) : each image can be taken with a different cam
    cv::Mat m_m_H_i; // Homography : 2D Image Plane to 2D Mosaic Frame (only for 2D mosaicking model)
    cv::Mat m_m_H_i_metric; // same as m_H_i in metric frame

    // Those lever arm parameters belongs to the camera as it can be changed using pan & tilt
    // during the acquisition
    cv::Mat m_V_T_C;// Vehicule to Camera Translation
    cv::Mat m_V_R_C;// Vehicule to Camera Rotation

    double m_scaling_factor; // Images can be reduced to improve performances ( 0 < scalingFactor <= 1 )

    bool m_scaling_factor_is_set;
    bool m_K_is_set;

    // Image taken in the camera plane with the vehicule navigation
    NavImage *m_image;


};

} // namespace optical_mapping

#endif // OPTICAL_MAPPING_PROJECTIVE_CAMERA_H_
