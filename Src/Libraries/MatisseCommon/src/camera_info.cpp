#include "camera_info.h"

using namespace MatisseCommon;
using namespace cv;

CameraInfo::CameraInfo()
{

}

QString CameraInfo::camera_name() const
{
    return m_camera_name;
}

void CameraInfo::setCamera_name(const QString &camera_name)
{
    m_camera_name = camera_name;
}

int CameraInfo::distortionModel() const
{
    return m_distortion_model;
}

void CameraInfo::setDistortionModel(int distortion_model)
{
    m_distortion_model = distortion_model;
}

cv::Mat CameraInfo::distortionCoeff() const
{
    return m_distortion_coeff;
}

void CameraInfo::setDistortionCoeff(const cv::Mat &distortion_coeff)
{
    m_distortion_coeff = distortion_coeff;
}

cv::Mat CameraInfo::K() const
{
    return m_K;
}

void CameraInfo::setK(const cv::Mat &K)
{
    m_K = K;
}

cv::Mat CameraInfo::vehicleToCameraTransform() const
{
    return m_vehicle_to_camera_transform;
}

void CameraInfo::setVehicleToCameraTransform(const cv::Mat &vehicle_to_camera_transform)
{
    m_vehicle_to_camera_transform = vehicle_to_camera_transform;
}

void CameraInfo::fullSensorSize(int &_width, int &_height)
{
    _width=m_full_sensor_width;
    _height=m_full_sensor_height;
}

void CameraInfo::setFullSensorSize(int &_width, int &_height)
{
    m_full_sensor_width = _width;
    m_full_sensor_height = _height;
}

bool CameraInfo::readFromFile(QString &_file_path)
{
    FileStorage fs(_file_path.toStdString(), FileStorage::READ);

    std::string temp_name;

    fs["camera_name"] >> temp_name; m_camera_name = QString::fromStdString(temp_name);
    fs["full_sensor_width"] >> m_full_sensor_width;
    fs["full_sensor_height"] >> m_full_sensor_height;
    fs["K"] >> m_K;
    fs["distortion_model"] >> m_distortion_model;
    fs["distortion_coeff"] >> m_distortion_coeff;
    fs["vehicle_to_camera_transform"] >> m_vehicle_to_camera_transform;

    std::cout << " m_K = "<< m_K <<std::endl;
    std::cout << " m_distortion_coeff = "<< m_distortion_coeff <<std::endl;

    fs.release();

    if (m_camera_name.isEmpty() || m_full_sensor_width==0.0 || m_full_sensor_height==0.0 || m_K.at<double>(2,2)!=1.0)
        return false;
    else
        return true;

}

bool CameraInfo::writeToFile(QString &_file_path)
{
    FileStorage fs(_file_path.toStdString(), FileStorage::WRITE);

    if (!fs.isOpened())
        return false;

    fs << "camera_name" << m_camera_name.toStdString();
    fs << "full_sensor_width" << m_full_sensor_width;
    fs << "full_sensor_height" << m_full_sensor_height;
    fs << "K" << m_K;
    fs << "distortion_model" << m_distortion_model;
    fs << "distortion_coeff" << m_distortion_coeff;
    fs << "vehicle_to_camera_transform" << m_vehicle_to_camera_transform;

    fs.release();

    return true;
}
