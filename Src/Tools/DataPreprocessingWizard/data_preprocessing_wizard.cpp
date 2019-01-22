#include "data_preprocessing_wizard.h"
#include "ui_data_preprocessing_wizard.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QRegExp>
#include <QProgressDialog>
#include <cmath>

DataPreprocessingWizard::DataPreprocessingWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::DataPreprocessingWizard)
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

    for (int i=0; i<m_found_files.size(); i++)
    {
        QString input_file_path = m_data_path+QDir::separator()+m_found_files[i];
        QFileInfo video_file_info(m_found_files[i]);
        QString output_path = ui->out_data_path_line->text()+QDir::separator()+video_file_info.baseName()+"_%05d.jpg";
        command_line="C:\\ffmpeg\\bin\\ffmpeg.exe -i %1 -vf \"fps=fps=%2:start_time=-%3:round=near\" -qscale 1 -qmin 1 -y  -f image2 %4";

        command_line = command_line.arg(input_file_path).arg(fps).arg(start_time).arg(output_path);
        qDebug() << command_line;
        ffmpeg_process.start(command_line);

        QProgressDialog progress("Extracting images files...", "Abort extraction", 0, 100, this);
        progress.setWindowModality(Qt::WindowModal);

        QRegExp duration_rex(".+Duration: (\\d+):(\\d+):(\\d+).+");
        QRegExp current_time_rex(".+time=(\\d+):(\\d+):(\\d+).+");
        double total_duration=-1,current_time=-1;

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

                foreach (QString image_file, images_files) {
                    QRegExp img_file_rex(".+_(\\d+)");

                    if (image_file.contains(img_file_rex))
                    {
                        double image_nb = img_file_rex.cap(1).toDouble();
                        double SSS = (double)SS+ (image_nb-1)*ui->extract_rate_sb->value();

                        QString new_file_name = QString("%1%2%3T%4%5%6Z.jpg")
                                .arg(yyyy).arg(mm,2, 10, QChar('0')).arg(dd,2, 10, QChar('0'))
                                .arg(HH,2, 10, QChar('0')).arg(MM,2, 10, QChar('0')).arg(SSS,6,'f',3,QChar('0'));
                        QFile image_qfile(processed_directory.absoluteFilePath(image_file));
                        image_qfile.rename(processed_directory.absoluteFilePath(new_file_name));
                    }
                }

            }
        }

    }
}

void DataPreprocessingWizard::sl_finished(int _state)
{
    qDebug() << "finished state : " << _state;

    if (_state == 1)
    {
        if (m_data_type == "Video")
        {
            video2Images(); // transform video to images
        }
    }
}
