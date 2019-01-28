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
                                                    tr("Open nav file"), "./", tr("Nav Files (*.txt)"));

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
    QStringList nameFilter = {"*.avi", "*.mp4", "*.ts", "*.mpg"};
    QDir directory(_dataPath);
    m_found_files = directory.entryList(nameFilter);


    if (m_found_files.size()==0)
    {
        this->back();
        QMessageBox::critical(this, tr("Alert"),
                              tr("No video file (*.avi, *.mp4, *.ts, *.mpg) found in this path"));
    }
    else
    {
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
    QStringList nameFilter = {"*.jpg","*.jpeg","*.png"};
    QDir directory(_dataPath);
    m_found_files = directory.entryList(nameFilter);

    if (m_found_files.size()==0)
    {
        this->back();
        QMessageBox::critical(this, tr("Alert"),
                              tr("No photo file (*.jpg , *.jpeg , *.png) found in this path"));
    }
    else
    {
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

        // Create ffmpeg command & start
        QString input_file_path = m_data_path+QDir::separator()+m_found_files[i];
        QFileInfo video_file_info(m_found_files[i]);
        QString output_path = ui->out_data_path_line->text()+QDir::separator()+video_file_info.baseName()+"_%05d.jpg";
        command_line="C:\\ffmpeg\\bin\\ffmpeg.exe -i %1 -vf \"fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 %4";

        command_line = command_line.arg(input_file_path).arg(fps).arg(start_time).arg(output_path);
        qDebug() << command_line;
        ffmpeg_process.start(command_line);

        QProgressDialog progress(QString("Extracting images files for video %1/%2").arg(i+1).arg(m_found_files.size()), "Abort extraction", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);

        QRegExp duration_rex(".+Duration: (\\d+):(\\d+):(\\d+).+");
        QRegExp current_time_rex(".+time=(\\d+):(\\d+):(\\d+).+");
        double total_duration=-1,current_time=-1;

        // Monitor ffmpeg process
        while(ffmpeg_process.waitForReadyRead(-1)){

            //QString output = ffmpeg_process.readAllStandardOutput();
            QString output = ffmpeg_process.readAllStandardError();

            if (output.contains(duration_rex))
            {
                total_duration = duration_rex.cap(1).toDouble()*3600+duration_rex.cap(2).toDouble()*60+duration_rex.cap(3).toDouble();
            }

            if (output.contains(current_time_rex))
            {
                current_time = current_time_rex.cap(1).toDouble()*3600+current_time_rex.cap(2).toDouble()*60+current_time_rex.cap(3).toDouble();
                progress.setValue(round(100*current_time/total_duration));
            }

            if (progress.wasCanceled())
                ffmpeg_process.kill();

            qDebug() << output;
        }

        // Check video filename format and rename images accordingly
        QRegExp video_stamped_rex("(.+)_(.+)_(\\d+)_(\\d+)\\.(.+)");

        if (m_found_files[i].contains(video_stamped_rex))
        {
            if ( video_stamped_rex.captureCount() == 5 )
            {
                QString date_time = video_stamped_rex.cap(3);
                int yyyy = 2000 + date_time.mid(0,2).toInt();
                int mm = date_time.mid(2,2).toInt();
                int dd = date_time.mid(4,2).toInt();
                int HH = date_time.mid(6,2).toInt();
                int MM = date_time.mid(8,2).toInt();
                int SS = date_time.mid(10,2).toInt();

                QStringList nameFilter = {"*.jpg"};
                QDir processed_directory(ui->out_data_path_line->text());
                QStringList images_files = processed_directory.entryList(nameFilter);

                // Check images files corresponding to video
                foreach (QString image_file, images_files) {
                    QRegExp img_file_rex(".+_(\\d+)");

                    if (image_file.contains(img_file_rex))
                    {
                        // base time
                        QDate date(yyyy,mm,dd);
                        QTime time( HH,MM,SS );

                        // add elapsed time
                        double image_nb = img_file_rex.cap(1).toDouble();
                        double past_SS =(image_nb-1)*ui->extract_rate_sb->value();
                        double past_SS_trun = floor(past_SS);
                        double past_MS = (past_SS-past_SS_trun)*1000;
                        time=time.addSecs((int)past_SS_trun);
                        time=time.addMSecs((int)past_MS);

                        // rename image with date and time format (iso)
                        QString new_file_name = QString("%1T%2Z.jpg").arg(date.toString("yyyyMMdd")).arg(time.toString("hhmmss.zzz"));
                        QFile image_qfile(processed_directory.absoluteFilePath(image_file));
                        image_qfile.rename(processed_directory.absoluteFilePath(new_file_name));

                        // if nav_file write out_nav_file
                        if (nav_out_file.isOpen())
                        {
                            QDateTime date_time(date,time);
                            double date_time_double = ((double)date_time.toMSecsSinceEpoch());

                            QString dim2_line("0;%1;%2;video;video;%3;%4;%5;%6;-99;%7;%8;%9;;;;;;;;;;;;;;;;;\n");
                            dim2_line = dim2_line.arg(date.toString("dd/MM/yyyy"))
                                    .arg(time.toString("hh:mm:ss.zzz"))
                                    .arg(new_file_name)
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
            }
        }
        if (nav_out_file.isOpen())
            nav_out_file.close();
    }
}

void DataPreprocessingWizard::sl_finished(int _state)
{
    qDebug() << "finished state : " << _state;

    if (_state == 1)
    {
        QString nav_file_path = ui->nav_file_line->text();
        if(!nav_file_path.isEmpty())
            m_nav_file = new NavFileReader(nav_file_path);

        if (m_data_type == "Video")
        {
            video2Images(); // transform video to images
        }
    }
}
