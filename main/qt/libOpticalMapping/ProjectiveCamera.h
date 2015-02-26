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
    ProjectiveCamera(NavImage *image_p, cv::Mat cameraMatrixK_p, cv::Mat V_T_C, cv::Mat V_R_C, qreal scaleFactor_p);

    // **** Attributes setters & getters ****

    cv::Mat K() const;
    void setK(const cv::Mat &K);

    cv::Mat m_H_i() const;
    void set_m_H_i(const cv::Mat &m_H_i);

    cv::Mat V_T_C() const;
    void setV_T_C(const cv::Mat &V_T_C);

    cv::Mat V_R_C() const;
    void setV_R_C(const cv::Mat &V_R_C);

    NavImage *image() const;
    void setImage(MatisseCommon::NavImage *image);

    qreal scalingFactor() const;
    void setScalingFactor(const qreal &scalingFactor);

    // **************************************

    ///
    /// \brief projectPtOnMosaickingPlane project a camera plane point to mosaic plane point
    /// \param camPlanePt_p camera plane point
    /// \param mosaicPlanePt_p mosaic plane point
    ///
    void projectPtOnMosaickingPlane(const cv::Mat camPlanePt_p, cv::Mat & mosaicPlanePt_p);

    ///
    /// \brief projectImageOnMosaickingPlane project camera plane image to mosaic plane point
    /// \param camPlanePt_p camera plane point
    /// \param mosaicPlanePt_p mosaic plane point
    ///
    void projectImageOnMosaickingPlane(cv::Mat & mosaicPlaneImage_p, cv::Mat & mosaicPlaneMask_p, cv::Point & corner_p);

private:

    cv::Mat _K; // Camera calibration matrix (intrinsics) : each image can be taken with a different cam
    cv::Mat _m_H_i; // Homography : 2D Image Plane to 2D Mosaic Frame

    // Those lever arm parameters belongs to the camera as it can be changed using pan & tilt
    // during the acquisition
    cv::Mat _V_T_C;// Vehicule to Camera Translation
    cv::Mat _V_R_C;// Vehicule to Camera Rotation

    qreal _scalingFactor; // Images can be reduced to improve performances ( 0 < scalingFactor <= 1 )

    bool _scalingFactorIsSet;
    bool _KIsSet;

    // Image taken in the camera plane with the vehicule navigation
    MatisseCommon::NavImage *_image;


};

#endif // PROJECTIVECAMERA_H
