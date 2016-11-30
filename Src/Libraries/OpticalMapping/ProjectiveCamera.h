#ifndef PROJECTIVECAMERA_H
#define PROJECTIVECAMERA_H

#include "libopticalmapping_global.h"

#include <opencv2/opencv.hpp>
#include <NavImage.h>

using namespace MatisseCommon;

class LIBOPTICALMAPPINGSHARED_EXPORT ProjectiveCamera
{
public:
    ProjectiveCamera();
    ProjectiveCamera(NavImage *image_p, cv::Mat cameraMatrixK_p, cv::Mat V_T_C, cv::Mat V_R_C, double scaleFactor_p);

    // **** Attributes setters & getters ****

    cv::Mat K() const;
    void setK(const cv::Mat &K);

    cv::Mat m_H_i() const;
    void set_m_H_i(const cv::Mat &m_H_i);

    cv::Mat m_H_i_metric() const;
    void setM_H_i_metric(const cv::Mat &m_H_i_metric);

    cv::Mat V_T_C() const;
    void setV_T_C(const cv::Mat &V_T_C);

    cv::Mat V_R_C() const;
    void setV_R_C(const cv::Mat &V_R_C);

    NavImage *image() const;
    void setImage(MatisseCommon::NavImage *image);

    double scalingFactor() const;
    void setScalingFactor(const double &scalingFactor);

    // **************************************

    ///
    /// \brief projectPtOnMosaickingPlane project a camera plane point to mosaic plane point
    /// \param camPlanePt_p camera plane point
    /// \param mosaicPlanePt_p mosaic plane point
    /// \param metric use metric frame if set to true
    ///
    void projectPtOnMosaickingPlane(const cv::Mat camPlanePt_p, cv::Mat & mosaicPlanePt_p, bool metric_p=false);

    ///
    /// \brief projectImageOnMosaickingPlane project camera plane image to mosaic plane point
    /// \param camPlanePt_p camera plane point
    /// \param mosaicPlanePt_p mosaic plane point
    ///
    void projectImageOnMosaickingPlane(cv::Mat & mosaicPlaneImage_p, cv::Mat & mosaicPlaneMask_p, cv::Point & corner_p);

    ///
    /// \brief computeImageExtent compute image origin and size
    /// \param corner_p origin of the image in the mosaicking frame
    /// \param dstSize size of the image projected on the mosaicking plane
    ///
    void computeImageExtent(cv::Point &corner_p, cv::Size &dstSize_p);

    ///
    /// \brief computeImageFootPrint compute image footprint coords on the mosaicking plane
    /// \param xArray x coords
    /// \param yArray y coords
    ///
    void computeImageFootPrint(std::vector<double> &xArray, std::vector<double> &yArray);


private:

    cv::Mat _K; // Camera calibration matrix (intrinsics) : each image can be taken with a different cam
    cv::Mat _m_H_i; // Homography : 2D Image Plane to 2D Mosaic Frame (only for 2D mosaicking model)
    cv::Mat _m_H_i_metric; // same as m_H_i in metric frame

    // Those lever arm parameters belongs to the camera as it can be changed using pan & tilt
    // during the acquisition
    cv::Mat _V_T_C;// Vehicule to Camera Translation
    cv::Mat _V_R_C;// Vehicule to Camera Rotation

    double _scalingFactor; // Images can be reduced to improve performances ( 0 < scalingFactor <= 1 )

    bool _scalingFactorIsSet;
    bool _KIsSet;

    // Image taken in the camera plane with the vehicule navigation
    MatisseCommon::NavImage *_image;


};

#endif // PROJECTIVECAMERA_H
