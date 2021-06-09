#ifndef MATISSE_CAMERA_INFO_H_
#define MATISSE_CAMERA_INFO_H_

#include <QString>
#include <opencv2/opencv.hpp>

namespace matisse {

class CameraInfo
{
public:
    CameraInfo();
    QString cameraName() const;
    void setCameraName(const QString &_camera_name);

    int distortionModel() const;
    void setDistortionModel(int _distortion_model);

    cv::Mat distortionCoeff() const;
    void setDistortionCoeff(const cv::Mat &_distortion_coeff);

    cv::Mat K() const;
    void setK(const cv::Mat &_K);

    cv::Mat vehicleToCameraTransform() const;
    void setVehicleToCameraTransform(const cv::Mat &_vehicle_to_camera_transform);

    void fullSensorSize(int &_width, int &_height);
    void setFullSensorSize(int &_width, int &_height);

    bool readFromFile(QString &_file_path);
    bool writeToFile(QString &_file_path);

    QString toQString();
    bool fromQString(QString _value);

private:
    QString m_camera_name;
    int m_distortion_model;
    cv::Mat m_distortion_coeff;
    cv::Mat m_K;
    cv::Mat m_vehicle_to_camera_transform;
    int m_full_sensor_width;
    int m_full_sensor_height;
};

} // namespace matisse

#endif // MATISSE_CAMERA_INFO_H_
