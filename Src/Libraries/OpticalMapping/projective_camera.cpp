#include "projective_camera.h"
#include <QDebug>

using namespace cv;
using namespace matisse_image;

namespace optical_mapping {

ProjectiveCamera::ProjectiveCamera():_scalingFactor(1),_scalingFactorIsSet(false),
    _KIsSet(false)
{

}

ProjectiveCamera::ProjectiveCamera(NavImage *image_p, Mat cameraMatrixK_p, Mat V_T_C, Mat V_R_C, qreal scaleFactor_p)
{

    this->setImage(image_p);
    this->setScalingFactor(scaleFactor_p);
    this->setK(cameraMatrixK_p);
    this->setV_T_C(V_T_C);
    this->setV_R_C(V_R_C);

}
cv::Mat ProjectiveCamera::K() const
{
    return _K;
}

void ProjectiveCamera::setK(const cv::Mat &K)
{
    Q_ASSERT(_scalingFactorIsSet);
    _K = _scalingFactor*K;
    _K.at<qreal>(2,2) = 1;
    qDebug() << "Last K component ) " << _K.at<qreal>(2,2) ;
    _KIsSet = true;
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

void ProjectiveCamera::setImage(NavImage *image)
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
    _scalingFactorIsSet = true;

}

void ProjectiveCamera::projectPtOnMosaickingPlane(const Mat camPlanePt_p, Mat &mosaicPlanePt_p, bool metric_p)
{
    if (metric_p == true){
        mosaicPlanePt_p = _m_H_i_metric * camPlanePt_p;
    }else{
        mosaicPlanePt_p = _m_H_i * camPlanePt_p;
    }
}

void ProjectiveCamera::projectImageOnMosaickingPlane(UMat &mosaicPlaneImage_p, UMat &mosaicPlaneMask_p, cv::Point & corner_p)
{


    cv::Size dstSize;

    computeImageExtent(corner_p, dstSize);

    // Project image on mosaic
    cv::Mat H = (cv::Mat_<qreal>(3,3) <<1, 0, -corner_p.x,0, 1, -corner_p.y,0, 0, 1)*_m_H_i;

    cv::warpPerspective(*(image()->imageData()), mosaicPlaneImage_p, H, dstSize, INTER_LINEAR, BORDER_REFLECT);

    // Project mask corresponding to images
    cv::Mat imageMask;
    imageMask.create(image()->imageData()->size(), CV_8U);
    imageMask.setTo(Scalar::all(255));

    cv::warpPerspective(imageMask, mosaicPlaneMask_p, H, dstSize);

}

void ProjectiveCamera::computeImageExtent(Point &corner_p, Size &dstSize_p)
{
    std::vector<qreal> xArray, yArray;
    std::vector<qreal>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    computeImageFootPrint(xArray, yArray);

    // Compute min,max
    min_x_it = std::min_element(xArray.begin(), xArray.end());
    min_y_it = std::min_element(yArray.begin(), yArray.end());
    max_x_it = std::max_element(xArray.begin(), xArray.end());
    max_y_it = std::max_element(yArray.begin(), yArray.end());

    // Compute dstSize
    int dstWidth = ceil(*max_x_it)-floor(*min_x_it) + 1;
    int dstHeight = ceil(*max_y_it)-floor(*min_y_it) + 1;

    cv::Size dstSize(dstWidth,dstHeight);
    dstSize_p = dstSize;

    corner_p.x = floor(*min_x_it);
    corner_p.y = floor(*min_y_it);

}

void ProjectiveCamera::computeImageFootPrint(std::vector<double> &xArray, std::vector<double> &yArray)
{
    cv::Mat pt1,pt2,pt3,pt4;

    int width = image()->width();
    int height = image()->height();


    // Project corners_p on mosaic plane
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   0,   1), pt1);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << width-1, 0,   1), pt2);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << width-1, height-1, 1), pt3);
    projectPtOnMosaickingPlane((cv::Mat_<qreal>(3,1) << 0,   height-1, 1), pt4);

    // Fill x & y array
    xArray.clear();
    yArray.clear();
    xArray.push_back(pt1.at<qreal>(0,0)/pt1.at<qreal>(2,0));
    yArray.push_back(pt1.at<qreal>(1,0)/pt1.at<qreal>(2,0));
    xArray.push_back(pt2.at<qreal>(0,0)/pt2.at<qreal>(2,0));
    yArray.push_back(pt2.at<qreal>(1,0)/pt2.at<qreal>(2,0));
    xArray.push_back(pt3.at<qreal>(0,0)/pt3.at<qreal>(2,0));
    yArray.push_back(pt3.at<qreal>(1,0)/pt3.at<qreal>(2,0));
    xArray.push_back(pt4.at<qreal>(0,0)/pt4.at<qreal>(2,0));
    yArray.push_back(pt4.at<qreal>(1,0)/pt4.at<qreal>(2,0));

}
cv::Mat ProjectiveCamera::m_H_i_metric() const
{
    return _m_H_i_metric;
}

void ProjectiveCamera::setM_H_i_metric(const cv::Mat &m_H_i_metric)
{
    _m_H_i_metric = m_H_i_metric;
}

} // namespace optical_mapping





