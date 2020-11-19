#include "camera_manager_tool.h"
#include "ui_camera_manager_tool.h"
#include <opencv2/opencv.hpp>
#include <QMessageBox>
#include <cmath>
#include "camera_info.h"

using namespace MatisseCommon;
using namespace cv;

CameraManagerTool::CameraManagerTool(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraManager)
{
    ui->setupUi(this);

    connect(ui->save_to_file_pb,SIGNAL(clicked()),this,SLOT(slot_saveCurrentCamera()));
}

CameraManagerTool::~CameraManagerTool()
{
    delete ui;
}

void CameraManagerTool::slot_saveCurrentCamera()
{
    Mat K(3, 3, CV_64F, 0.0);
    Mat dist_coeff, vehicle_to_cam_transform;
    bool valid;
    bool all_correct=true;

    // fill K
    K.at<double>(0,0)=ui->K11_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,0)=ui->K21_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,0)=ui->K31_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(0,1)=ui->K12_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,1)=ui->K22_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,1)=ui->K32_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(0,2)=ui->K13_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,2)=ui->K23_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,2)=ui->K33_le->text().toDouble(&valid); all_correct &= valid;

    if(!all_correct || K.at<double>(2,2)!=1.0 || K.at<double>(0,0)==0.0 || K.at<double>(1,1)==0.0 || K.at<double>(0,2)==0.0 || K.at<double>(1,2)==0.0)
    {
        QMessageBox::warning(this,tr("Invalid camera matrix"),tr("Camera cannot be saved as the camera matrix is wrong"));
        return;
    }

    // 0:radial k1, 1:radial k3, 2:radial+tan, 3:fisheye
    int distortion_model = ui->dist_model_cb->currentIndex();

    switch (distortion_model) {
    case 0:
        dist_coeff = Mat(1,1,CV_64F);
        dist_coeff.at<double>(0,0)=ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 1:
        dist_coeff = Mat(1,3,CV_64F);
        dist_coeff.at<double>(0,0)=ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 2:
        dist_coeff = Mat(1,5,CV_64F);
        dist_coeff.at<double>(0,0)=ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,3)=ui->d4_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,4)=ui->d5_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 3:
        dist_coeff = Mat(1,4,CV_64F);
        dist_coeff.at<double>(0,0)=ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,3)=ui->d4_le->text().toDouble(&valid); all_correct &= valid;
        break;

    }

    if(!all_correct)
    {
        QMessageBox::warning(this,tr("Invalid camera distortion"),tr("Camera cannot be saved as the distortion coefficients are wrong"));
        return;
    }

    // vehicle transform
    vehicle_to_cam_transform = Mat(1,6,CV_64F);
    vehicle_to_cam_transform.at<double>(0,0)=ui->X_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,1)=ui->Y_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,2)=ui->Z_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,3)=M_PI*ui->roll_le->text().toDouble(&valid)/180.0; all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,4)=M_PI*ui->pitch_le->text().toDouble(&valid)/180.0; all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,5)=M_PI*ui->yaw_le->text().toDouble(&valid)/180.0; all_correct &= valid;

    if(!all_correct || fabs(vehicle_to_cam_transform.at<double>(0,3))>M_PI || fabs(vehicle_to_cam_transform.at<double>(0,4))>M_PI || fabs(vehicle_to_cam_transform.at<double>(0,5))>M_PI)
    {
        QMessageBox::warning(this,tr("Invalid IMU to camera transform"),tr("IMU to camera transformation is wrong. If you don't have any idea of the transformation, leave 0 everywhere. Rotations are in degrees between -180 and 180."));
        return;
    }

    QString camera_name = ui->camera_name->text();

    if(camera_name.isEmpty())
    {
        QMessageBox::warning(this,tr("No camera name"),tr("You must give a name to your camera."));
        return;
    }

    int sensor_width = ui->sensor_width->text().toInt(&valid); all_correct &= valid;
    int sensor_height = ui->sensor_height->text().toInt(&valid); all_correct &= valid;

    if(!all_correct || sensor_width==0.0 || sensor_height==0.0)
    {
        QMessageBox::warning(this,tr("Invalid sensor size"),tr("You must give a valid sensor size. It is the size of the images used for calibration. If you then rescale your images, Matisse will handle the scaling"));
        return;
    }

    // Everything went well so we affect to CamInfo
    CameraInfo camera_info;

    camera_info.setK(K);
    camera_info.setCamera_name(camera_name);
    camera_info.setDistortionCoeff(dist_coeff);
    camera_info.setDistortionModel(distortion_model);
    camera_info.setVehicleToCameraTransform(vehicle_to_cam_transform);
    camera_info.setFullSensorSize(sensor_width,sensor_height);

    QString dest_file = QString("/home/ifremer/cam.yaml");
    camera_info.readFromFile(dest_file);

}
