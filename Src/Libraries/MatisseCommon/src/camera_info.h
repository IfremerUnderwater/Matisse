#ifndef CAMERAINFO_H
#define CAMERAINFO_H

#include <QString>
#include <opencv2/opencv.hpp>

namespace MatisseCommon {

class CameraInfo
{
public:
    CameraInfo();
    QString cameraName() const;
    void setCameraName(const QString &cameraName);

    int distortionModel() const;
    void setDistortionModel(int distortionModel);

    cv::Mat distortionCoeff() const;
    void setDistortionCoeff(const cv::Mat &distortionCoeff);

    cv::Mat K() const;
    void setK(const cv::Mat &K);

    cv::Mat vehicleToCameraTransform() const;
    void setVehicleToCameraTransform(const cv::Mat &vehicleToCameraTransform);

    void fullSensorSize(int &_width, int &_height);
    void setFullSensorSize(int &_width, int &_height);

    bool readFromFile(QString &_file_path);
    bool writeToFile(QString &_file_path);

private:
    QString m_camera_name;
    int m_distortion_model;
    cv::Mat m_distortion_coeff;
    cv::Mat m_K;
    cv::Mat m_vehicle_to_camera_transform;
    int m_full_sensor_width;
    int m_full_sensor_height;
};

}

#endif // CAMERAINFO_H
