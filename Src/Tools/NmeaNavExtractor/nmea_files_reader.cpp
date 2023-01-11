#include "nmea_files_reader.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <QString>
#define _USE_MATH_DEFINES
#include <math.h>
#include <cmath>

double DEG2RAD=M_PI/180;


NmeaFilesReader::NmeaFilesReader(QFileInfoList _nmea_files_path):m_nmea_files_info(_nmea_files_path),m_is_valid(false)
{
    m_lat_interpolator=nullptr;
    m_lon_interpolator=nullptr;
    m_depth_interpolator=nullptr;
    m_alt_interpolator = nullptr;
    m_roll_interpolator=nullptr;
    m_pitch_interpolator=nullptr;
    m_yaw_interpolator=nullptr;

    if(loadFilesToMemory())
        m_is_valid=true;
}

NmeaFilesReader::~NmeaFilesReader()
{
    if(m_lat_interpolator)
        delete m_lat_interpolator;
    if(m_lon_interpolator)
        delete m_lon_interpolator;
    if(m_depth_interpolator)
        delete m_depth_interpolator;
    if (m_alt_interpolator)
        delete m_alt_interpolator;
    if(m_roll_interpolator)
        delete m_roll_interpolator;
    if(m_pitch_interpolator)
        delete m_pitch_interpolator;
    if(m_yaw_interpolator)
        delete m_yaw_interpolator;
}

bool NmeaFilesReader::loadFilesToMemory()
{
    m_nav_datetime.clear();
    m_lat.clear();
    m_lon.clear();
    m_depth.clear();
    m_alt.clear();
    m_roll.clear();
    m_pitch.clear();
    m_yaw.clear();

    if( !m_nmea_files_info.isEmpty() )
    {
        for (int i=0; i<m_nmea_files_info.size(); i++)
        {

            QFile file(m_nmea_files_info[i].absoluteFilePath());
            if(!file.open(QIODevice::ReadOnly)) {
                return false;
            }

            QTextStream file_stream(&file);

            while(!file_stream.atEnd()) {

                QString line = file_stream.readLine();
                QStringList fields_com = line.split(",");

                if(fields_com.size()>=5)
                {
                    // get hrov.nav msf2
                    if (fields_com[4]==QString("hrov.nav"))
                    {
                        QDate nav_date= QDate::fromString(fields_com[1],"dd/MM/yyyy");
                        QTime time = QTime::fromString(fields_com[2],"hh:mm:ss.zzz");
                        QDateTime date_time(nav_date,time);
                        m_nav_datetime.push_back((double)date_time.toMSecsSinceEpoch());

                        std::pair<double, double> pair;

                        pair.first = (double)date_time.toMSecsSinceEpoch();

                        pair.second = fields_com[6].mid(9,-1).toDouble();
                        m_lat.push_back(pair);

                        pair.second = fields_com[7].mid(10,-1).toDouble();
                        m_lon.push_back(pair);

                        pair.second = fields_com[8].mid(6,-1).toDouble();
                        m_depth.push_back(pair);

                        pair.second = fields_com[9].mid(9,-1).toDouble();
                        m_alt.push_back(pair);

                        pair.second = DEG2RAD*fields_com[10].mid(8,-1).toDouble();
                        m_yaw.push_back(pair);

                        pair.second = DEG2RAD*fields_com[11].mid(5,-1).toDouble();;
                        m_roll.push_back(pair);

                        pair.second = DEG2RAD*fields_com[12].mid(6,-1).toDouble();;
                        m_pitch.push_back(pair);
                    }

                    // get victor.nav msf2
                    if (fields_com[4]==QString("victor.nav") && !fields_com[2].endsWith(":60.000"))
                    {

                        QDate nav_date= QDate::fromString(fields_com[1],"dd/MM/yyyy");
                        QTime time = QTime::fromString(fields_com[2],"hh:mm:ss.zzz");
                        QDateTime date_time(nav_date,time);
                        m_nav_datetime.push_back((double)date_time.toMSecsSinceEpoch());

                        std::pair<double, double> pair;

                        pair.first = (double)date_time.toMSecsSinceEpoch();

                        pair.second = fields_com[6].mid(9,12).toDouble();
                        m_lat.push_back(pair);

                        pair.second = fields_com[7].mid(10,12).toDouble();
                        m_lon.push_back(pair);

                        pair.second = fields_com[8].mid(6,7).toDouble();
                        m_depth.push_back(pair);

                        pair.second = fields_com[9].mid(9, 7).toDouble();
                        m_alt.push_back(pair);

                        pair.second = DEG2RAD*fields_com[10].mid(8,5).toDouble();
                        m_yaw.push_back(pair);

                        pair.second = 0.0;
                        m_pitch.push_back(pair);

                        pair.second = 0.0;
                        m_roll.push_back(pair);

                    }

                    // get victor.Cameras msf2
                    if (fields_com[4]==QString("victor.Cameras"))
                    {
                        QDate nav_date= QDate::fromString(fields_com[1],"dd/MM/yyyy");
                        QTime time = QTime::fromString(fields_com[2],"hh:mm:ss.zzz");
                        QDateTime date_time(nav_date,time);
                        m_nav_datetime.push_back((double)date_time.toMSecsSinceEpoch());

                        std::pair<double, double> pair;

                        pair.first = (double)date_time.toMSecsSinceEpoch();

                        pair.second = fields_com[6].mid(4,6).toDouble();
                        m_pan.push_back(pair);

                        pair.second = fields_com[7].mid(5,6).toDouble();
                        m_tilt.push_back(pair);

                        pair.second = fields_com[8].mid(5,4).toDouble();
                        m_zoom.push_back(pair);

                    }
                }
            }

            file.close();

        }

        m_lat_interpolator = new Interpolator(m_lat);
        m_lon_interpolator = new Interpolator(m_lon);
        m_depth_interpolator = new Interpolator(m_depth);
        m_alt_interpolator = new Interpolator(m_alt);
        m_yaw_interpolator = new AngleInterpolator(m_yaw);
        m_pitch_interpolator = new AngleInterpolator(m_pitch);
        m_roll_interpolator = new AngleInterpolator(m_roll);

        m_pan_interpolator = new Interpolator(m_pan);
        m_tilt_interpolator = new Interpolator(m_tilt);
        m_zoom_interpolator = new Interpolator(m_zoom);

        return true;
    }
    else
    {
        return false;
    }
}

bool NmeaFilesReader::isValid() const
{
    return m_is_valid;
}

double NmeaFilesReader::latAtTime(double &_time)
{
    return m_lat_interpolator->findValue(_time);
}

double NmeaFilesReader::lonAtTime(double &_time)
{
    return m_lon_interpolator->findValue(_time);
}

double NmeaFilesReader::depthAtTime(double &_time)
{
    return m_depth_interpolator->findValue(_time);
}

double NmeaFilesReader::altAtTime(double& _time)
{
    return m_alt_interpolator->findValue(_time);
}

double NmeaFilesReader::rollAtTime(double &_time)
{
    return m_roll_interpolator->findValue(_time);
}

double NmeaFilesReader::pitchAtTime(double &_time)
{
    return m_pitch_interpolator->findValue(_time);
}

double NmeaFilesReader::yawAtTime(double &_time)
{
    return m_yaw_interpolator->findValue(_time);
}

double NmeaFilesReader::panAtTime(double &_time)
{
    return m_pan_interpolator->findValue(_time);
}

double NmeaFilesReader::tiltAtTime(double &_time)
{
    return m_tilt_interpolator->findValue(_time);
}

double NmeaFilesReader::zoomAtTime(double &_time)
{
    return m_zoom_interpolator->findValue(_time);
}

std::vector<double>& NmeaFilesReader::getNavDateTime()
{
    return m_nav_datetime;
}
