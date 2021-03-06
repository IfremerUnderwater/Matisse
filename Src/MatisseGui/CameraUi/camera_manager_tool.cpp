#include "camera_manager_tool.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>
#include "ui_camera_manager_tool.h"
#include <opencv2/opencv.hpp>
#include <QMessageBox>
#include "camera_info.h"
#include "camera_manager.h"

using namespace cv;

namespace matisse {

CameraManagerTool::CameraManagerTool(QWidget *_parent) :
    QDialog(_parent),
    m_ui(new Ui::CameraManagerTool)
{
    m_ui->setupUi(this);

    connect(m_ui->save_to_file_pb,SIGNAL(clicked()),this,SLOT(sl_saveCurrentCamera()));
    connect(m_ui->delete_from_db_pb,SIGNAL(clicked()),this,SLOT(sl_deleteCurrentCamera()));

    CameraManager *cam_manager_p = &(CameraManager::instance());
    connect(cam_manager_p,SIGNAL(si_cameraListChanged()),this,SLOT(sl_refreshCameraList()));

    connect(m_ui->cam_selection_cb,SIGNAL(currentTextChanged(QString)),this,SLOT(sl_cameraSelected(QString)));
    connect(m_ui->dist_model_cb,SIGNAL(currentIndexChanged(int)),this,SLOT(sl_distModelChanged(int)));

    sl_refreshCameraList();

    m_ui->dist_model_cb->setCurrentIndex(1);
    sl_distModelChanged(1);

}

CameraManagerTool::~CameraManagerTool()
{
    delete m_ui;
}

void CameraManagerTool::sl_refreshCameraList()
{
    m_ui->cam_selection_cb->clear();
    m_ui->cam_selection_cb->addItem("New camera");
    m_ui->cam_selection_cb->addItems(CameraManager::instance().cameraList());
}

void CameraManagerTool::sl_saveCurrentCamera()
{
    Mat K(3, 3, CV_64F, 0.0);
    Mat dist_coeff, vehicle_to_cam_transform;
    bool valid;
    bool all_correct=true;

    // fill K
    K.at<double>(0,0)=m_ui->K11_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,0)=m_ui->K21_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,0)=m_ui->K31_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(0,1)=m_ui->K12_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,1)=m_ui->K22_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,1)=m_ui->K32_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(0,2)=m_ui->K13_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(1,2)=m_ui->K23_le->text().toDouble(&valid); all_correct &= valid;
    K.at<double>(2,2)=m_ui->K33_le->text().toDouble(&valid); all_correct &= valid;

    if(!all_correct || K.at<double>(2,2)!=1.0 || K.at<double>(0,0)==0.0 || K.at<double>(1,1)==0.0 || K.at<double>(0,2)==0.0 || K.at<double>(1,2)==0.0)
    {
        QMessageBox::warning(this,tr("Invalid camera matrix"),tr("Camera cannot be saved as the camera matrix is wrong"));
        return;
    }

    // 0:radial k1, 1:radial k3, 2:radial+tan, 3:fisheye
    int distortion_model = m_ui->dist_model_cb->currentIndex();

    switch (distortion_model) {
    case 0:
        dist_coeff = Mat(1,1,CV_64F);
        dist_coeff.at<double>(0,0)=m_ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 1:
        dist_coeff = Mat(1,3,CV_64F);
        dist_coeff.at<double>(0,0)=m_ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=m_ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=m_ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 2:
        dist_coeff = Mat(1,5,CV_64F);
        dist_coeff.at<double>(0,0)=m_ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=m_ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=m_ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,3)=m_ui->d4_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,4)=m_ui->d5_le->text().toDouble(&valid); all_correct &= valid;
        break;
    case 3:
        dist_coeff = Mat(1,4,CV_64F);
        dist_coeff.at<double>(0,0)=m_ui->d1_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,1)=m_ui->d2_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,2)=m_ui->d3_le->text().toDouble(&valid); all_correct &= valid;
        dist_coeff.at<double>(0,3)=m_ui->d4_le->text().toDouble(&valid); all_correct &= valid;
        break;

    }

    if(!all_correct)
    {
        QMessageBox::warning(this,tr("Invalid camera distortion"),tr("Camera cannot be saved as the distortion coefficients are wrong"));
        return;
    }

    // vehicle transform
    vehicle_to_cam_transform = Mat(1,6,CV_64F);
    vehicle_to_cam_transform.at<double>(0,0)=m_ui->X_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,1)=m_ui->Y_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,2)=m_ui->Z_le->text().toDouble(&valid); all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,3)=M_PI*m_ui->roll_le->text().toDouble(&valid)/180.0; all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,4)=M_PI*m_ui->pitch_le->text().toDouble(&valid)/180.0; all_correct &= valid;
    vehicle_to_cam_transform.at<double>(0,5)=M_PI*m_ui->yaw_le->text().toDouble(&valid)/180.0; all_correct &= valid;

    if(!all_correct || fabs(vehicle_to_cam_transform.at<double>(0,3))>M_PI || fabs(vehicle_to_cam_transform.at<double>(0,4))>M_PI || fabs(vehicle_to_cam_transform.at<double>(0,5))>M_PI)
    {
        QMessageBox::warning(this,tr("Invalid IMU to camera transform"),tr("IMU to camera transformation is wrong. If you don't have any idea of the transformation, leave 0 everywhere. Rotations are in degrees between -180 and 180."));
        return;
    }

    QString camera_name = m_ui->camera_name->text();

    if(camera_name.isEmpty() || camera_name=="Unknown")
    {
        QMessageBox::warning(this,tr("No camera name"),tr("You must give a name to your camera. Note that \"Unknown\" is a reserved name that you cannot use"));
        return;
    }

    int sensor_width = m_ui->sensor_width->text().toInt(&valid); all_correct &= valid;
    int sensor_height = m_ui->sensor_height->text().toInt(&valid); all_correct &= valid;

    if(!all_correct || sensor_width==0.0 || sensor_height==0.0)
    {
        QMessageBox::warning(this,tr("Invalid sensor size"),tr("You must give a valid sensor size. It is the size of the images used for calibration. If you then rescale your images, Matisse will handle the scaling"));
        return;
    }

    // Everything went well so we affect to CamInfo
    CameraInfo camera_info;

    camera_info.setK(K);
    camera_info.setCameraName(camera_name);
    camera_info.setDistortionCoeff(dist_coeff);
    camera_info.setDistortionModel(distortion_model);
    camera_info.setVehicleToCameraTransform(vehicle_to_cam_transform);
    camera_info.setFullSensorSize(sensor_width,sensor_height);

    if(m_ui->cam_selection_cb->currentText() != "New camera")
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Overwriting camera"), tr("You are about to overwrite the current camera. Are you sure ?"),
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    // add it to manager and write to file
    CameraManager::instance().addCamera(camera_info);

    QMessageBox::information(this,tr("Camera saved to database"),tr("Your camera has been added to database in the folder ")
                             +CameraManager::instance().camInfoDir().absolutePath()+tr(". You can save this file appart and share it with other users if you like."));

}

void CameraManagerTool::sl_deleteCurrentCamera()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Delete camera"), tr("You are about to delete %1 camera. Are you sure ?").arg(m_ui->cam_selection_cb->currentText()),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No)
        return;

    if( CameraManager::instance().deleteCameraByName(m_ui->cam_selection_cb->currentText()) )
    {
        QMessageBox::information(this,tr("Camera removed database"),tr("This camera has been deleted."));
    }
    else
    {
        QMessageBox::warning(this,tr("Cannot remove camera from database"),tr("That is not possible to remove a camera that has not been saved in database !"));
    }
}

void CameraManagerTool::sl_cameraSelected(QString _selected_item)
{
    if(_selected_item.isEmpty())
        return;

    CameraInfo current_cam;

    if(_selected_item != "New camera")
        current_cam = CameraManager::instance().cameraByName(_selected_item);

    // fill K
    m_ui->K11_le->setText(QString::number(current_cam.K().at<double>(0,0)));
    m_ui->K21_le->setText(QString::number(current_cam.K().at<double>(1,0)));
    m_ui->K31_le->setText(QString::number(current_cam.K().at<double>(2,0)));
    m_ui->K12_le->setText(QString::number(current_cam.K().at<double>(0,1)));
    m_ui->K22_le->setText(QString::number(current_cam.K().at<double>(1,1)));
    m_ui->K32_le->setText(QString::number(current_cam.K().at<double>(2,1)));
    m_ui->K13_le->setText(QString::number(current_cam.K().at<double>(0,2)));
    m_ui->K23_le->setText(QString::number(current_cam.K().at<double>(1,2)));
    m_ui->K33_le->setText(QString::number(current_cam.K().at<double>(2,2)));

    // 0:radial k1, 1:radial k3, 2:radial+tan, 3:fisheye
    m_ui->dist_model_cb->setCurrentIndex(current_cam.distortionModel());

    switch (current_cam.distortionModel()) {
    case 0:
        m_ui->d1_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,0)));
        m_ui->d2_le->setText(QString::number(0.0));
        m_ui->d3_le->setText(QString::number(0.0));
        m_ui->d4_le->setText(QString::number(0.0));
        m_ui->d5_le->setText(QString::number(0.0));
        break;
    case 1:
        m_ui->d1_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,0)));
        m_ui->d2_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,1)));
        m_ui->d3_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,2)));
        m_ui->d4_le->setText(QString::number(0.0));
        m_ui->d5_le->setText(QString::number(0.0));
        break;
    case 2:
        m_ui->d1_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,0)));
        m_ui->d2_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,1)));
        m_ui->d3_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,2)));
        m_ui->d4_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,3)));
        m_ui->d5_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,4)));
        break;
    case 3:
        m_ui->d1_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,0)));
        m_ui->d2_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,1)));
        m_ui->d3_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,2)));
        m_ui->d4_le->setText(QString::number(current_cam.distortionCoeff().at<double>(0,3)));
        m_ui->d5_le->setText(QString::number(0.0));
        break;

    }

    // vehicle transform
    m_ui->X_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,0)/M_PI));
    m_ui->Y_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,1)/M_PI));
    m_ui->Z_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,2)/M_PI));
    m_ui->roll_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,3)/M_PI));
    m_ui->pitch_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,4)/M_PI));
    m_ui->yaw_le->setText(QString::number(180*current_cam.vehicleToCameraTransform().at<double>(0,5)/M_PI));

    int sensor_width,sensor_height;
    current_cam.fullSensorSize(sensor_width,sensor_height);
    m_ui->sensor_width->setText( QString::number(sensor_width));
    m_ui->sensor_height->setText( QString::number(sensor_height));

    m_ui->camera_name->setText(current_cam.cameraName());

}

void CameraManagerTool::sl_distModelChanged(int _dist_model)
{
    switch (_dist_model) {
    case 0:
        m_ui->d1_le->setText(QString::number(0.0)); m_ui->d1_le->setEnabled(true);
        m_ui->d2_le->setText(""); m_ui->d2_le->setEnabled(false);
        m_ui->d3_le->setText(""); m_ui->d3_le->setEnabled(false);
        m_ui->d4_le->setText(""); m_ui->d4_le->setEnabled(false);
        m_ui->d5_le->setText(""); m_ui->d5_le->setEnabled(false);
        break;
    case 1:
        m_ui->d1_le->setText(QString::number(0.0)); m_ui->d1_le->setEnabled(true);
        m_ui->d2_le->setText(QString::number(0.0)); m_ui->d2_le->setEnabled(true);
        m_ui->d3_le->setText(QString::number(0.0)); m_ui->d3_le->setEnabled(true);
        m_ui->d4_le->setText(""); m_ui->d4_le->setEnabled(false);
        m_ui->d5_le->setText(""); m_ui->d5_le->setEnabled(false);
        break;
    case 2:
        m_ui->d1_le->setText(QString::number(0.0)); m_ui->d1_le->setEnabled(true);
        m_ui->d2_le->setText(QString::number(0.0)); m_ui->d2_le->setEnabled(true);
        m_ui->d3_le->setText(QString::number(0.0)); m_ui->d3_le->setEnabled(true);
        m_ui->d4_le->setText(QString::number(0.0)); m_ui->d4_le->setEnabled(true);
        m_ui->d5_le->setText(QString::number(0.0)); m_ui->d5_le->setEnabled(true);
        break;
    case 3:
        m_ui->d1_le->setText(QString::number(0.0)); m_ui->d1_le->setEnabled(true);
        m_ui->d2_le->setText(QString::number(0.0)); m_ui->d2_le->setEnabled(true);
        m_ui->d3_le->setText(QString::number(0.0)); m_ui->d3_le->setEnabled(true);
        m_ui->d4_le->setText(QString::number(0.0)); m_ui->d4_le->setEnabled(true);
        m_ui->d5_le->setText(""); m_ui->d5_le->setEnabled(false);
        break;

    }
}

} // namespace matisse
