#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

#include "nmea_extractor_wizard.h"
#include "ui_nmea_extractor_wizard.h"

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

double RAD2DEG=180/M_PI;

NmeaExtractorWizard::NmeaExtractorWizard(QWidget *parent) :
    QWizard(parent),
    ui(new Ui::NmeaExtractorWizard),
    m_nmea_file_reader(NULL)
{
    ui->setupUi(this);

    connect(ui->select_nmea_path, SIGNAL(released()), this, SLOT(sl_selectNmeaPath()));
    connect(ui->select_nav_file, SIGNAL(released()), this, SLOT(sl_selectNavOutputFile()));

    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(sl_pageChanged(int)));
    connect(this,SIGNAL(finished(int)),this,SLOT(sl_finished(int)));


}

NmeaExtractorWizard::~NmeaExtractorWizard()
{
    delete ui;
    if (m_nmea_file_reader)
        delete m_nmea_file_reader;
}

void NmeaExtractorWizard::sl_selectNmeaPath()
{
    QString data_folder = QFileDialog::getExistingDirectory(this, tr("Nmea Data path"),
                                                            "./",
                                                            QFileDialog::ShowDirsOnly
                                                            | QFileDialog::DontResolveSymlinks);
    if (!data_folder.isEmpty())
    {
        if (QDir::separator()=="\\")
            data_folder.replace("/","\\");

        ui->nmea_path_line->setText(data_folder);
    }
}

void NmeaExtractorWizard::sl_selectNavOutputFile()
{
    QString output_nav_file = QFileDialog::getSaveFileName(this, "Select path and file name for nav file");

    if (!output_nav_file.isEmpty())
    {
        if (QDir::separator()=="\\")
            output_nav_file.replace("/","\\");
        ui->nav_output_file->setText(output_nav_file);
    }
}


void NmeaExtractorWizard::sl_pageChanged(int _page_idx)
{
    switch (_page_idx) {
    case 0:

        break;
    case 1:
        break;
    case 2:

        break;
    default:
        break;
    }
}


void NmeaExtractorWizard::checkAndFillNmeaFiles(QString _dataPath)
{
    QStringList nameFilter = {"*.nme"};
    QDir directory(_dataPath);
    m_found_nmea_files = directory.entryInfoList(nameFilter);

    if (m_found_nmea_files.size()==0)
    {
        this->back();
        QMessageBox::critical(this, tr("Alert"),
                              tr("No nmea file (*.nme) found in this path"));
    }
    else {
        m_nmea_file_reader = new NmeaFilesReader(m_found_nmea_files);
    }

}

void NmeaExtractorWizard::generateNavFile(QFileInfo _nav_file_info)
{
    // Create nav file
    QFile nav_file;
    QTextStream nav_file_stream;

    QString nav_filename = _nav_file_info.dir().path() + QDir::separator() + _nav_file_info.baseName() + QString(".txt");
    nav_file.setFileName(nav_filename);

    if (nav_file.open(QIODevice::WriteOnly))
        nav_file_stream.setDevice(&nav_file);
    else{
        QMessageBox::critical(this, tr("Alert"),
            tr("Cannot create destination file !"));
        return;
    }

    // Create header
    nav_file_stream << "dd/MM/yyyy, hh:mm:ss.zzz, lat, lon, depth, alt, yaw, pitch, roll\n";

    double first_nav_time = m_nmea_file_reader->getNavDateTime()[0];
    double last_nav_time = m_nmea_file_reader->getNavDateTime()[m_nmea_file_reader->getNavDateTime().size()-1];

    double current_nav_time = first_nav_time;

    while (current_nav_time < last_nav_time)
    {
        QDateTime current_nav_qdatetime;
        current_nav_qdatetime.setMSecsSinceEpoch((qint64)current_nav_time);

        QString line = QString("%1, %2, %3, %4, %5, %6, %7, %8\n")
            .arg(current_nav_qdatetime.toString("dd/MM/yyyy,hh:mm:ss.zzz"))
            .arg(m_nmea_file_reader->latAtTime(current_nav_time), 0, 'f', 8)
            .arg(m_nmea_file_reader->lonAtTime(current_nav_time), 0, 'f', 8)
            .arg(m_nmea_file_reader->depthAtTime(current_nav_time), 0, 'f', 2)
            .arg(m_nmea_file_reader->altAtTime(current_nav_time), 0, 'f', 2)
            .arg(RAD2DEG * m_nmea_file_reader->yawAtTime(current_nav_time), 0, 'f', 1)
            .arg(RAD2DEG * m_nmea_file_reader->pitchAtTime(current_nav_time), 0, 'f', 1)
            .arg(RAD2DEG * m_nmea_file_reader->rollAtTime(current_nav_time), 0, 'f', 1);

        nav_file_stream << line;

        current_nav_time += 500.0;
    }

    /*int video_duration = static_cast<int>( this->getVideoDuration(_nav_file_info.absoluteFilePath()) );

    if (video_duration >0)
    {
        // get starting video time
        QString basename = QString("2019-")+_nav_file_info.baseName();
        this->monthLettersToNumber(basename);
        basename.replace("-SDI4K-A_","");
        QDateTime base_date_time = QDateTime::fromString(basename,"yyyyMMdd_hh-mm-ss");

        // Create subtitle file
        QFile subtitle_file;
        QTextStream subtitle_file_stream;

        QString subtitle_filename = _nav_file_info.dir().path() + QDir::separator() + _nav_file_info.baseName() + QString(".ssa");
        subtitle_file.setFileName(subtitle_filename);
        if (subtitle_file.open(QIODevice::WriteOnly))
            subtitle_file_stream.setDevice(&subtitle_file);

        // Write header
        subtitle_file_stream << "[Script Info]\n";
        subtitle_file_stream << QString("Title: %1\n").arg(_nav_file_info.baseName());
        subtitle_file_stream << "Original Script: Video4K\n";
        subtitle_file_stream << "ScriptType: v4.00\n";
        subtitle_file_stream << "Collisions: Normal\n";
        subtitle_file_stream << "PlayResY: 1080\n";
        subtitle_file_stream << "PlayDepth: 0\n";
        subtitle_file_stream << "Timer: 100,0000\n\n";

        subtitle_file_stream << "[V4 Styles]\n";
        subtitle_file_stream << "Format: Name, Fontname, Fontsize, PrimaryColour, SecondaryColour, TertiaryColour, BackColour, Bold, Italic, BorderStyle, Outline, Shadow, Alignment, MarginL, MarginR, MarginV, AlphaLevel, Encoding\n\n";

        subtitle_file_stream << "Style: Clock,Courier New,42,65535,65535,65535,-2147483640,1,0,1,1,0,1,30,00,30,0,0\n\n";

        subtitle_file_stream << "[Events]\n";
        subtitle_file_stream << "Format: Marked, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text\n\n";

        for (int i=0;i<=video_duration; i++) {
            QDateTime current_nav_qdatetime = base_date_time.addSecs(i);
            double base_date_time_shifted_double = ((double)current_nav_qdatetime.toMSecsSinceEpoch());

            QTime start_time(0,0);
            start_time = start_time.addSecs(i);
            QTime end_time(0,0);
            end_time = end_time.addSecs(i+1);

            QString line1 = QString("%1 Pan=%2° Tilt=%3° Zoom=%4x")
                    .arg(current_nav_qdatetime.toString("hh:mm:ss"))
                    .arg(m_nmea_file_reader->panAtTime(base_date_time_shifted_double),0,'f',1)
                    .arg(m_nmea_file_reader->tiltAtTime(base_date_time_shifted_double),0,'f',1)
                    .arg(m_nmea_file_reader->zoomAtTime(base_date_time_shifted_double),0,'f',1);

            QString line2 = QString("Lat=%1° Lon=%2° Depth=%3 Heading=%4°")
                    .arg(m_nmea_file_reader->latAtTime(base_date_time_shifted_double),0,'f',8)
                    .arg(m_nmea_file_reader->lonAtTime(base_date_time_shifted_double),0,'f',8)
                    .arg(m_nmea_file_reader->depthAtTime(base_date_time_shifted_double),0,'f',2)
                    .arg(RAD2DEG*m_nmea_file_reader->yawAtTime(base_date_time_shifted_double),0,'f',1);

            subtitle_file_stream << i+1 << "\n";
            subtitle_file_stream << QString("Dialogue: Marked=0,%1.00,%2.00,Clock,,0000,0000,0000,,%3\n").arg(start_time.toString("hh:mm:ss")).arg(end_time.toString("hh:mm:ss")).arg(line1);
            subtitle_file_stream << QString("Dialogue: Marked=0,%1.00,%2.00,Clock,,0000,0100,0000,,%3\n\n").arg(start_time.toString("hh:mm:ss")).arg(end_time.toString("hh:mm:ss")).arg(line2);


        }

        if (subtitle_file.isOpen())
            subtitle_file.close();

        // generate snapshots
        this->video2Snapshot(_nav_file_info,base_date_time,video_duration);

    }*/
}


void NmeaExtractorWizard::sl_finished(int _state)
{
    qDebug() << "finished state : " << _state;

    if (_state == 1)
    {
        this->checkAndFillNmeaFiles(ui->nmea_path_line->text());

        QFileInfo nav_file(ui->nav_output_file->text());

        if (!ui->nav_output_file->text().isEmpty())
        {
            this->generateNavFile(nav_file);
        }
        else {
            QMessageBox::critical(this, tr("Alert"),
                tr("Destination nav file is empty !"));
        }


    }
    QMessageBox::information(this, tr("End"),
                             tr("Processing ended"));
    QApplication::quit();

}
