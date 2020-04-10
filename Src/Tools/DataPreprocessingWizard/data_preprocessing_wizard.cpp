#include <cmath>

#include "data_preprocessing_wizard.h"
#include "ui_data_preprocessing_wizard.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QProgressDialog>
#include <QTextStream>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QImageReader>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "imageprocessing.h"

double RAD2DEG=180/M_PI;

DataPreprocessingWizard::DataPreprocessingWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DataPreprocessingWizard),
    m_nav_file(NULL)
{
    ui->setupUi(this);

    connect(ui->select_path, SIGNAL(released()), this, SLOT(sl_selectDataPath()));
    connect(ui->select_nav_file, SIGNAL(released()), this, SLOT(sl_selectNavFile()));
    connect(ui->select_all, SIGNAL(released()), this, SLOT(sl_selectAllFiles()));
    connect(ui->deselect_all, SIGNAL(released()), this, SLOT(sl_deselectAllFiles()));
    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(sl_pageChanged(int)));
    connect(this,SIGNAL(finished(int)),this,SLOT(sl_finished(int)));


}

DataPreprocessingWizard::~DataPreprocessingWizard()
{
    delete ui;
    if (m_nav_file)
        delete m_nav_file;
}

void DataPreprocessingWizard::sl_selectDataPath()
{
    QString data_folder = QFileDialog::getExistingDirectory(this, tr("Data path"),
                                                            "./",
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
    if (!data_folder.isEmpty())
    {
        if (QDir::separator()=="\\")
            data_folder.replace("/","\\");

        ui->data_path_line->setText(data_folder);
        ui->out_data_path_line->setText(data_folder+QDir::separator()+"processed_images");
    }
}

void DataPreprocessingWizard::sl_selectOutputPath()
{
    QString output_folder = QFileDialog::getExistingDirectory(this, tr("Output path"),
                                                              "./",
                                                              QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks);
    if (!output_folder.isEmpty())
    {
        if (QDir::separator()=="\\")
            output_folder.replace("/","\\");
        ui->out_data_path_line->setText(output_folder);
    }
}

void DataPreprocessingWizard::sl_selectNavFile()
{
    QString nav_file = QFileDialog::getOpenFileName(this,
                                                    tr("Open nav file"), "./", tr("Nav Files (*.txt *.csv)"));

    if (!nav_file.isEmpty())
    {
        if (QDir::separator()=="\\")
            nav_file.replace("/","\\");
        ui->nav_file_line->setText(nav_file);
    }
}

void DataPreprocessingWizard::sl_pageChanged(int _page_idx)
{
    switch (_page_idx) {
    case 0:

        break;
    case 1:
        if(!ui->data_path_line->text().isEmpty())
        {
            m_data_path = ui->data_path_line->text();
            m_data_type = ui->data_type_combo->currentText();
            if (m_data_type == "Video")
                checkAndFillVideoFiles(m_data_path);
            else
                checkAndFillPhotoFiles(m_data_path);
        }else
        {
            this->back();
            QMessageBox::critical(this, tr("Alert"),
                                  tr("You have to fill a datapath !"));
        }
        break;
    case 2:

        break;
    default:
        break;
    }
}

void DataPreprocessingWizard::sl_selectAllFiles()
{
    for (int i = 0; i < ui->files_list->count(); i++) {
        QListWidgetItem *item = ui->files_list->item(i);
        item->setCheckState(Qt::Checked);
    }
}

void DataPreprocessingWizard::sl_deselectAllFiles()
{
    for (int i = 0; i < ui->files_list->count(); i++) {
        QListWidgetItem *item = ui->files_list->item(i);
        item->setCheckState(Qt::Unchecked);
    }
}

void DataPreprocessingWizard::checkAndFillVideoFiles(QString _dataPath)
{
    QStringList nameFilter = {"*.avi", "*.mp4", "*.ts", "*.mpg", "*.mov", "*.mxf", "*.mkv"};
    QDir directory(_dataPath);
    m_found_files = directory.entryList(nameFilter);


    if (m_found_files.size()==0)
    {
        this->back();
        QMessageBox::critical(this, tr("Alert"),
                              tr("No video file (*.avi, *.mp4, *.ts, *.mpg, *.mov, *.mxf , *.mkv) found in this path"));
    }
    else
    {
        ui->files_list->clear();
        ui->files_list->addItems(m_found_files);
        for (int i = 0; i < ui->files_list->count(); i++) {
            QListWidgetItem *item = ui->files_list->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
            item->setCheckState(Qt::Checked); // AND initialize check state
        }
    }

}

void DataPreprocessingWizard::checkAndFillPhotoFiles(QString _dataPath)
{
    QStringList nameFilter = {"*.jpg","*.jpeg","*.png", "*.tiff", "*.tif"};
    QDir directory(_dataPath);
    m_found_files = directory.entryList(nameFilter);

    if (m_found_files.size()==0)
    {
        this->back();
        QMessageBox::critical(this, tr("Alert"),
                              tr("No photo file (*.jpg , *.jpeg , *.png, *.tiff, *.tif) found in this path"));
    }
    else
    {
        ui->files_list->clear();
        ui->files_list->addItems(m_found_files);
        for (int i = 0; i < ui->files_list->count(); i++) {
            QListWidgetItem *item = ui->files_list->item(i);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable); // set checkable flag
            item->setCheckState(Qt::Checked); // AND initialize check state
        }
    }

}

void DataPreprocessingWizard::video2Images()
{
    double fps = 1.0/ui->extract_rate_sb->value();
    double start_time = (ui->extract_rate_sb->value())/2-0.00001; // this fix ffmpeg start shift
    QProcess ffmpeg_process;
    ffmpeg_process.setWorkingDirectory(m_data_path);

    QString command_line;
    QFile nav_out_file;
    QTextStream nav_out_file_stream;

    if(m_nav_file->isValid())
    {
        QFileInfo nav_in_file(ui->nav_file_line->text());
        QString nav_out_filename = ui->out_data_path_line->text() + QDir::separator()
                + nav_in_file.baseName() + QString(".dim2");
        nav_out_file.setFileName(nav_out_filename);
        if (nav_out_file.open(QIODevice::WriteOnly))
            nav_out_file_stream.setDevice(&nav_out_file);
    }

    // loop on all video files
    for (int i=0; i<m_found_files.size(); i++)
    {

        // Check if video is to process or not
        if (ui->files_list->item(i)->checkState()==Qt::Checked)
        {

            // Create ffmpeg command & start
            QString input_file_path = m_data_path+QDir::separator()+m_found_files[i];
            QFileInfo video_file_info(m_found_files[i]);
            QString output_path = ui->out_data_path_line->text()+QDir::separator()+video_file_info.baseName()+"_%05d.jpg";

#ifdef WIN32
            if(ui->deinterlace_video_cb->isChecked())
                command_line="ffmpeg.exe -i \"%1\" -vf \"yadif=0,fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 \"%4\"";
            else
                command_line="ffmpeg.exe -i \"%1\" -vf \"fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 \"%4\"";
#else
            if(ui->deinterlace_video_cb->isChecked())
                command_line="ffmpeg -i \"%1\" -vf \"yadif=0,fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 \"%4\"";
            else
                command_line="ffmpeg -i \"%1\" -vf \"fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 \"%4\"";
#endif
            command_line = command_line.arg(input_file_path).arg(fps).arg(start_time).arg(output_path);
            qDebug() << command_line;
            ffmpeg_process.start(command_line);

            QProgressDialog ffmpeg_progress(QString("Extracting images files for video %1/%2").arg(i+1).arg(m_found_files.size()), "Abort extraction", 0, 100, this);
            ffmpeg_progress.setWindowModality(Qt::WindowModal);

            QRegExp duration_rex(".+Duration: (\\d+):(\\d+):(\\d+).+");
            QRegExp current_time_rex(".+time=(\\d+):(\\d+):(\\d+).+");
            double total_duration=-1,current_time=-1;

            // Monitor ffmpeg process
            while(ffmpeg_process.waitForReadyRead(-1)){

                //QString output = ffmpeg_process.readAllStandardOutput();
                QString output = ffmpeg_process.readAllStandardOutput() + ffmpeg_process.readAllStandardError();

                if (output.contains(duration_rex))
                {
                    total_duration = duration_rex.cap(1).toDouble()*3600+duration_rex.cap(2).toDouble()*60+duration_rex.cap(3).toDouble();
                }

                if (output.contains(current_time_rex))
                {
                    current_time = current_time_rex.cap(1).toDouble()*3600+current_time_rex.cap(2).toDouble()*60+current_time_rex.cap(3).toDouble();
                    ffmpeg_progress.setValue(round(100*current_time/total_duration));
                }

                if (ffmpeg_progress.wasCanceled())
                {
                    ffmpeg_process.kill();
                    return;
                }

                qDebug() << output;
            }

            // Check video filename formats, rename images accordingly and preprocess
            QRegExp video_stamped_format1_rex("(.+)_(.+)_(\\d+)_(\\d+)\\.(.+)");
            QRegExp video_stamped_format2_rex("(.+)T(.+)Z.(.+)");
            bool valid_video_stamp1 = false;
            bool valid_video_stamp2 = false;

            if (m_found_files[i].contains(video_stamped_format1_rex))
                valid_video_stamp1=true;
            if (m_found_files[i].contains(video_stamped_format2_rex))
                valid_video_stamp2=true;

            if (valid_video_stamp1 || valid_video_stamp2)
            {
                // base time
                QDate base_date;
                QTime base_time;
                QDateTime base_date_time;

                if (valid_video_stamp1)
                {
                    QString string_date_time = video_stamped_format1_rex.cap(3);
                    int yyyy = 2000 + string_date_time.mid(0,2).toInt();
                    int mm = string_date_time.mid(2,2).toInt();
                    int dd = string_date_time.mid(4,2).toInt();
                    int HH = string_date_time.mid(6,2).toInt();
                    int MM = string_date_time.mid(8,2).toInt();
                    int SS = string_date_time.mid(10,2).toInt();

                    base_date.setDate(yyyy,mm,dd);
                    base_time.setHMS(HH,MM,SS);
                }
                else
                {
                    base_date = QDate::fromString(video_stamped_format2_rex.cap(1),"yyyyMMdd");
                    base_time = QTime::fromString(video_stamped_format2_rex.cap(2),"hhmmss.zzz");
                }

                // set base date time
                base_date_time.setDate(base_date);
                base_date_time.setTime(base_time);

                QStringList nameFilter = {"*.jpg"};
                QDir processed_directory(ui->out_data_path_line->text());
                QStringList images_files = processed_directory.entryList(nameFilter);

                // progressbar for preprocessing
                QProgressDialog prepro_progress(QString("Preprocessing images files"), "Abort processing", 0, 100, this);
                prepro_progress.setWindowModality(Qt::WindowModal);
                int j=1;

                // Check images files corresponding to video
                foreach (QString image_file, images_files) {
                    QRegExp img_file_rex(".+_(\\d+)");

                    if (prepro_progress.wasCanceled())
                        return;

                    if (image_file.contains(img_file_rex))
                    {
                        // add elapsed time
                        double image_nb = img_file_rex.cap(1).toDouble();
                        double past_SS =(image_nb-1)*ui->extract_rate_sb->value();
                        double past_SS_trun = floor(past_SS);
                        double past_MS = (past_SS-past_SS_trun)*1000;
                        QDateTime date_time_img=base_date_time.addSecs((int)past_SS_trun);
                        date_time_img=date_time_img.addMSecs((int)past_MS);

                        // update date and time
                        QDate date_img=date_time_img.date();
                        QTime time_img=date_time_img.time();

                        // rename image with date and time format (iso)
                        QString new_file_name = QString("%1T%2Z.jpg").arg(date_img.toString("yyyyMMdd")).arg(time_img.toString("hhmmss.zzz"));
                        QFile image_qfile(processed_directory.absoluteFilePath(image_file));
                        image_qfile.remove(processed_directory.absoluteFilePath(new_file_name));
                        image_qfile.rename(processed_directory.absoluteFilePath(new_file_name));

                        // preprocess if needed
                        preprocessImage(processed_directory.absoluteFilePath(new_file_name),
                                        processed_directory.absoluteFilePath(new_file_name));

                        // if nav_file write out_nav_file
                        if (nav_out_file.isOpen())
                        {
                            double date_time_double = ((double)date_time_img.toMSecsSinceEpoch());

                            QString dim2_line("0;%1;%2;video;video;%3;%4;%5;%6;%7;%8;%9;%10;;;;;;;;;;;;;;;;;\n");
                            dim2_line = dim2_line.arg(date_img.toString("dd/MM/yyyy"))
                                    .arg(time_img.toString("hh:mm:ss.zzz"))
                                    .arg(new_file_name)
                                    .arg(m_nav_file->latAtTime(date_time_double),0,'f',10)
                                    .arg(m_nav_file->lonAtTime(date_time_double),0,'f',10)
                                    .arg(m_nav_file->depthAtTime(date_time_double),0,'f',3)
                                    .arg(m_nav_file->altAtTime(date_time_double),0,'f',3)
                                    .arg(m_nav_file->yawAtTime(date_time_double)*RAD2DEG,0,'f',2)
                                    .arg(m_nav_file->rollAtTime(date_time_double)*RAD2DEG,0,'f',2)
                                    .arg(m_nav_file->pitchAtTime(date_time_double)*RAD2DEG,0,'f',2);
                            nav_out_file_stream << dim2_line;
                        }
                    }
                    prepro_progress.setValue(round(100*j/images_files.size()));
                    j++;
                }
            }
            else
            {
                QMessageBox::information(this,"Video file naming","The video file you provided does not match any of the supported naming rules. Your model won't be georeferenced and scaled if you use those images");
            }

        }
    }
    if (nav_out_file.isOpen())
        nav_out_file.close();
}

void DataPreprocessingWizard::preprocessImages()
{
    QStringList nameFilter = {"*.jpg","*.jpeg","*.png", "*.tiff", "*.tif"};
    QDir data_dir(ui->data_path_line->text());
    QDir out_data_dir(ui->out_data_path_line->text());
    QStringList images_files = data_dir.entryList(nameFilter);

    QFile nav_out_file;
    QTextStream nav_out_file_stream;

    if(m_nav_file->isValid())
    {
        QFileInfo nav_in_file(ui->nav_file_line->text());
        QString nav_out_filename = ui->out_data_path_line->text() + QDir::separator()
                + nav_in_file.baseName() + QString(".dim2");
        nav_out_file.setFileName(nav_out_filename);
        if (nav_out_file.open(QIODevice::WriteOnly))
            nav_out_file_stream.setDevice(&nav_out_file);
    }

    // progressbar for preprocessing
    QProgressDialog prepro_progress(QString("Preprocessing images files"), "Abort processing", 0, 100, this);
    prepro_progress.setWindowModality(Qt::WindowModal);
    int j=1;

    // process images
    foreach (QString image_file, images_files) {

        if (prepro_progress.wasCanceled())
            return;

        // preprocess if needed
        preprocessImage(data_dir.absoluteFilePath(image_file),
                        out_data_dir.absoluteFilePath(image_file));

        // regenerate nav if needed and possible
        // if nav_file write out_nav_file
        if (nav_out_file.isOpen())
        {

            QRegExp photo_stamped_rex("(\\d+)T(\\d+)\\.(\\d+)Z\\.(.+)");
            if (image_file.contains(photo_stamped_rex))
            {
                if ( photo_stamped_rex.captureCount() == 4 )
                {
                    QDate date = QDate::fromString(photo_stamped_rex.cap(1),"yyyyMMdd");
                    QTime time = QTime::fromString(photo_stamped_rex.cap(2)+"."+photo_stamped_rex.cap(3).mid(0,3),"hhmmss.zzz");
                    QDateTime date_time(date,time);

                    double date_time_double = ((double)date_time.toMSecsSinceEpoch());

                    QString dim2_line("0;%1;%2;video;video;%3;%4;%5;%6;-99;%7;%8;%9;;;;;;;;;;;;;;;;;\n");
                    dim2_line = dim2_line.arg(date.toString("dd/MM/yyyy"))
                            .arg(time.toString("hh:mm:ss.zzz"))
                            .arg(image_file)
                            .arg(m_nav_file->latAtTime(date_time_double),0,'f',10)
                            .arg(m_nav_file->lonAtTime(date_time_double),0,'f',10)
                            .arg(m_nav_file->depthAtTime(date_time_double),0,'f',3)
                            .arg(m_nav_file->yawAtTime(date_time_double)*RAD2DEG,0,'f',2)
                            .arg(m_nav_file->rollAtTime(date_time_double)*RAD2DEG,0,'f',2)
                            .arg(m_nav_file->pitchAtTime(date_time_double)*RAD2DEG,0,'f',2);
                    nav_out_file_stream << dim2_line;
                }
            }
        }
        prepro_progress.setValue(round(100*j/images_files.size()));
        j++;
    }

    if (nav_out_file.isOpen())
        nav_out_file.close();

}

void DataPreprocessingWizard::preprocessImage(QString _image_path, QString _out_image_path)
{
    if(ui->correct_colors_cb->isChecked() || ui->res_limit_cb->isChecked())
    {
        QImageReader qimg(_image_path);
        if (qimg.canRead())
        {
            cv::Mat cv_img;

            // reduce image if needed
            if(ui->res_limit_cb->isChecked()){
                int width = qimg.size().width();
                int height = qimg.size().height();
                double img_mpx = (double)(width*height)/1e6;

                if (img_mpx > ui->res_limit_sb->value())
                {
                    cv::Mat cv_temp_img = cv::imread(_image_path.toStdString());
                    cv::Size dst_size;
                    double mpx_ratio_sqrt = sqrt(ui->res_limit_sb->value()/img_mpx);
                    dst_size.width = round(mpx_ratio_sqrt*cv_temp_img.cols);
                    dst_size.height = round(mpx_ratio_sqrt*cv_temp_img.rows);
                    cv::resize(cv_temp_img,cv_img,dst_size);

                    if(!ui->correct_colors_cb->isChecked())
                        cv::imwrite(_out_image_path.toStdString(),cv_img);
                }
            }

            // color correct if needed
            if(ui->correct_colors_cb->isChecked())
            {
                cv::Mat cv_out_img,empty_mask;

                if(cv_img.empty())
                    cv_img = cv::imread(_image_path.toStdString());

                histogramQuantileStretch(cv_img, empty_mask, 0.0005,  cv_out_img);

                cv::imwrite(_out_image_path.toStdString(),cv_out_img);
            }

        }
    }
}

void DataPreprocessingWizard::sl_finished(int _state)
{
    qDebug() << "finished state : " << _state;

    if (_state == 1)
    {
        QDir outdir(ui->out_data_path_line->text());
        if(!outdir.exists())
            outdir.mkpath(outdir.absolutePath());

        QString nav_file_path = ui->nav_file_line->text();
        if(!nav_file_path.isEmpty())
            m_nav_file = new NavFileReader(nav_file_path);
        else
            m_nav_file = new NavFileReader("");

        if (m_data_type == "Video")
        {
            video2Images(); // transform video to images
        }
        if (m_data_type == "Photo")
        {
            preprocessImages();
        }
    }

    QMessageBox::information(this, tr("End"),
                             tr("Preprocessing ended"));
    QApplication::quit();

}
