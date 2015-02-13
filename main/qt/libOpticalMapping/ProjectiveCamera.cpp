#include "ProjectiveCamera.h"


ProjectiveCamera::ProjectiveCamera()
{

    _V_T_C.zeros(3,1,CV_64F);
    _V_R_C.zeros(3,3,CV_64F);

}
cv::Mat ProjectiveCamera::K() const
{
    return _K;
}

void ProjectiveCamera::setK(const cv::Mat &K)
{
    _K = _scalingFactor*K;
}
cv::Mat ProjectiveCamera::m_H_i() const
{
    return _m_H_i;
}

void ProjectiveCamera::set_m_H_i(const cv::Mat &m_H_i)
{
    _m_H_i = m_H_i;
}
cv::Mat ProjectiveCamera::V_T_C() const
{
    return _V_T_C;
}

void ProjectiveCamera::setV_T_C(const cv::Mat &V_T_C)
{
    _V_T_C = V_T_C;
}
cv::Mat ProjectiveCamera::V_R_C() const
{
    return _V_R_C;
}

void ProjectiveCamera::setV_R_C(const cv::Mat &V_R_C)
{
    _V_R_C = V_R_C;
}
NavImage *ProjectiveCamera::image() const
{
    return _image;
}

void ProjectiveCamera::setImage(MatisseCommon::NavImage *image)
{
    _image = image;
}
qreal ProjectiveCamera::scalingFactor() const
{
    return _scalingFactor;
}

void ProjectiveCamera::setScalingFactor(const qreal &scalingFactor)
{
    if (scalingFactor >= 1){
        _scalingFactor = 1;
    }else if (scalingFactor <= 0){
        exit(1);
    }else{
        _scalingFactor = scalingFactor;
    }

}

void ProjectiveCamera::projectPtOnMosaickingPlane(const Mat camPlanePt_p, Mat &mosaicPlanePt_p)
{
    mosaicPlanePt_p = _m_H_i * camPlanePt_p;
}






