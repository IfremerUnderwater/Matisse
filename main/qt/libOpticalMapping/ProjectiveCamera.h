#ifndef PROJECTIVECAMERA_H
#define PROJECTIVECAMERA_H

#include <opencv2/opencv.hpp>

class ProjectiveCamera
{
public:
    ProjectiveCamera();


    cv::Mat K() const;
    void setK(const cv::Mat &K);

    cv::Mat m_H_i() const;
    void setM_H_i(const cv::Mat &m_H_i);

private:

    cv::Mat _K; // Camera calibration matrix (intrinsics)
    cv::Mat _m_H_i; // Homography : 2D Image Plane to 2D Mosaic Frame

};

#endif // PROJECTIVECAMERA_H
