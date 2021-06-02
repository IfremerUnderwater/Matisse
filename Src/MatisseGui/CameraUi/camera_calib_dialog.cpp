#include "camera_calib_dialog.h"
#include "ui_camera_calib_dialog.h"
#include "camera_calib.h"
#include <QFileDialog>
#include <vector>
#include <string>
#include <algorithm>
#include <string>
#include <cctype>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include "camera_info.h"
#include "camera_manager.h"

using namespace matisse;

CameraCalibDialog::CameraCalibDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CameraCalibDialog)
{
    ui->setupUi(this);

    ui->dist_model_cb->setCurrentIndex(1);

    QObject::connect(ui->select_path_pb, SIGNAL(clicked()), this, SLOT(slot_onCalibPathSelection()));
    QObject::connect(ui->quit_pb, SIGNAL(clicked()), this, SLOT(hide()));
    QObject::connect(ui->calibrate_pb, SIGNAL(clicked()), this, SLOT(slot_onCalibrateCameras()));

}

CameraCalibDialog::~CameraCalibDialog()
{
    delete ui;
}

void CameraCalibDialog::slot_onCalibPathSelection()
{
    QString img_folder = QFileDialog::getExistingDirectory(this,
                                                    tr("Open calibration images path"),"./");
    ui->img_calib_path->setText(img_folder);
}

void CameraCalibDialog::slot_onCalibrateCameras()
{
    // Clear old calibration text
    ui->log_text_edit->clear();

    // Get and set camera name
    if (ui->camera_name_le->text().isEmpty())
    {
        QMessageBox::warning(this, tr("No camera name"), tr("You must give a name to your camera in order for it to be saved to database."));
        return;
    }

    CameraInfo cam_info;
    cam_info.setCameraName(ui->camera_name_le->text());

    // dist model
    cam_info.setDistortionModel(ui->dist_model_cb->currentIndex());

    // Get images
    QStringList nameFilter = {"*.jpg", "*.jpeg" ,"*.tiff" ,"*.png" };
    QDir data_dir(ui->img_calib_path->text());
    QStringList images_files = data_dir.entryList(nameFilter);

    if (images_files.size() == 0)
    {
        QMessageBox::warning(this, tr("No image found"), tr("No image have been found. Check that your dataset is in the right format (*.jpg, *.jpeg, *.tiff, *.png)"));
        return;
    }

    // Push them in final vector
    std::vector<std::string> images_files_std;
    for (int i=0;i< images_files.size();i++) {
        images_files_std.push_back(data_dir.absoluteFilePath(images_files[i]).toStdString());
    }

    // Calibrate the camera
    cv::Size board_size(ui->horiz_squares_nb->text().toInt(),ui->vertic_squares_nb->text().toInt());
    float squares_size = ui->squares_size->text().toFloat();

    if (board_size.width==0 || board_size.height==0 || squares_size<=0)
    {
        QMessageBox::warning(this, tr("Pattern issue"), tr("You must give valid pattern informations, meaning non zero board size and positive squares size."));
        return;
    }

    CameraCalib calib(images_files_std,board_size,squares_size,ui->log_text_edit);

    double reproj_error;
    calib.calibrateMono(cam_info, reproj_error, true);

    if (reproj_error<0)
    {
        QMessageBox::warning(this, tr("Calibration issue"), tr("Something wrong has happened, check the log."));
        return;
    }

    if (reproj_error > 5)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Poor quality calibration"), tr("Your rms calibration error is more than 5 pixels this usualy means the calibration is not good enough. Do you still want to add this camera to database ?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    // Push camera to database
    CameraManager::instance().addCamera(cam_info);

    ui->log_text_edit->append(tr("Your camera has been calibrated and added to database. You can find it in the camera manager"));

}

