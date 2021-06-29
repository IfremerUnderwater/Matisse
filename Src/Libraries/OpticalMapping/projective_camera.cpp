#include "projective_camera.h"
#include <QDebug>

using namespace cv;
using namespace matisse_image;

namespace optical_mapping {

ProjectiveCamera::ProjectiveCamera():
    m_scaling_factor(1),m_scaling_factor_is_set(false),
    m_K_is_set(false)
{

}

ProjectiveCamera::ProjectiveCamera(NavImage *_image_p, Mat _camera_matrix_K_p, Mat _V_T_C, Mat _V_R_C, qreal _scale_factor_p)
{

    this->setImage(_image_p);
    this->setScalingFactor(_scale_factor_p);
    this->setK(_camera_matrix_K_p);
    this->setV_T_C(_V_T_C);
    this->setV_R_C(_V_R_C);

}
cv::Mat ProjectiveCamera::K() const
{
    return m_K;
}

void ProjectiveCamera::setK(const cv::Mat &_K)
{
    Q_ASSERT(m_scaling_factor_is_set);
    m_K = m_scaling_factor*_K;
    m_K.at<qreal>(2,2) = 1;

    m_K_is_set = true;
}
cv::Mat ProjectiveCamera::m_H_i() const
{
    return m_m_H_i;
}

void ProjectiveCamera::set_m_H_i(const cv::Mat &_m_H_i)
{
    m_m_H_i = _m_H_i;
}
cv::Mat ProjectiveCamera::V_T_C() const
{
    return m_V_T_C;
}

void ProjectiveCamera::setV_T_C(const cv::Mat &_V_T_C)
{
    m_V_T_C = _V_T_C;
}
cv::Mat ProjectiveCamera::V_R_C() const
{
    return m_V_R_C;
}

void ProjectiveCamera::setV_R_C(const cv::Mat &_V_R_C)
{
    m_V_R_C = _V_R_C;
}
NavImage *ProjectiveCamera::image() const
{
    return m_image;
}

void ProjectiveCamera::setImage(NavImage *_image)
{
    m_image = _image;
}
qreal ProjectiveCamera::scalingFactor() const
{
    return m_scaling_factor;
}

void ProjectiveCamera::setScalingFactor(const qreal &_scaling_factor)
{
    if (_scaling_factor >= 1){
        m_scaling_factor = 1;
    }else if (_scaling_factor <= 0){
        m_scalingFactor = 1;
    }else{
        m_scaling_factor = _scaling_factor;
    }
    m_scaling_factor_is_set = true;

}

void ProjectiveCamera::projectPtOnMosaickingPlane(const Mat _cam_plane_pt_p, Mat &_mosaic_plane_pt_p, bool _metric_p)
{
    if (_metric_p == true){
        _mosaic_plane_pt_p = m_m_H_i_metric * _cam_plane_pt_p;
    }else{
        _mosaic_plane_pt_p = m_m_H_i * _cam_plane_pt_p;
    }
}

void ProjectiveCamera::projectImageOnMosaickingPlane(UMat &_mosaic_plane_image_p, UMat &mosaic_plane_mask_p, cv::Point & _corner_p)
{


    cv::Size _dst_size;

    computeImageExtent(_corner_p, _dst_size);

    // Project image on mosaic
    cv::Mat H = (cv::Mat_<qreal>(3,3) <<1, 0, -_corner_p.x,0, 1, -_corner_p.y,0, 0, 1)*m_m_H_i;

    cv::warpPerspective(*(image()->imageData()), _mosaic_plane_image_p, H, _dst_size, INTER_LINEAR, BORDER_REFLECT);

    // Project mask corresponding to images
    cv::Mat _image_mask;
    _image_mask.create(image()->imageData()->size(), CV_8U);
    _image_mask.setTo(Scalar::all(255));

    cv::warpPerspective(_image_mask, mosaic_plane_mask_p, H, _dst_size);

}

void ProjectiveCamera::computeImageExtent(Point &_corner_p, Size &_dst_size_p)
{
    std::vector<qreal> x_array, y_array;
    std::vector<qreal>::iterator min_x_it, min_y_it, max_x_it, max_y_it;

    computeImageFootPrint(x_array, y_array);

    // Compute min,max
    min_x_it = std::min_element(x_array.begin(), x_array.end());
    min_y_it = std::min_element(y_array.begin(), y_array.end());
    max_x_it = std::max_element(x_array.begin(), x_array.end());
    max_y_it = std::max_element(y_array.begin(), y_array.end());

    // Compute dstSize
    int dst_width = ceil(*max_x_it)-floor(*min_x_it) + 1;
    int dst_height = ceil(*max_y_it)-floor(*min_y_it) + 1;

    cv::Size dst_size(dst_width,dst_height);
    _dst_size_p = dst_size;

    _corner_p.x = floor(*min_x_it);
    _corner_p.y = floor(*min_y_it);

}

void ProjectiveCamera::computeImageFootPrint(std::vector<double> &_x_array, std::vector<double> &_y_array)
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
    _x_array.clear();
    _y_array.clear();
    _x_array.push_back(pt1.at<qreal>(0,0)/pt1.at<qreal>(2,0));
    _y_array.push_back(pt1.at<qreal>(1,0)/pt1.at<qreal>(2,0));
    _x_array.push_back(pt2.at<qreal>(0,0)/pt2.at<qreal>(2,0));
    _y_array.push_back(pt2.at<qreal>(1,0)/pt2.at<qreal>(2,0));
    _x_array.push_back(pt3.at<qreal>(0,0)/pt3.at<qreal>(2,0));
    _y_array.push_back(pt3.at<qreal>(1,0)/pt3.at<qreal>(2,0));
    _x_array.push_back(pt4.at<qreal>(0,0)/pt4.at<qreal>(2,0));
    _y_array.push_back(pt4.at<qreal>(1,0)/pt4.at<qreal>(2,0));

}
cv::Mat ProjectiveCamera::m_H_i_metric() const
{
    return m_m_H_i_metric;
}

void ProjectiveCamera::setM_H_i_metric(const cv::Mat &_m_H_i_metric)
{
    m_m_H_i_metric = _m_H_i_metric;
}

} // namespace optical_mapping





