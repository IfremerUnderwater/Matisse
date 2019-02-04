#include "nav_file_reader.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QTime>
#include <cmath>

double DEG2RAD=M_PI/180;


NavFileReader::NavFileReader(QString _file_path):m_file_path(_file_path),m_is_valid(false)
{
    if(loadFileToMemory())
        m_is_valid=true;
}

NavFileReader::~NavFileReader()
{
    if(m_lat_interpolator)
        delete m_lat_interpolator;
    if(m_lon_interpolator)
        delete m_lon_interpolator;
    if(m_depth_interpolator)
        delete m_depth_interpolator;
    if(m_roll_interpolator)
        delete m_roll_interpolator;
    if(m_pitch_interpolator)
        delete m_pitch_interpolator;
    if(m_yaw_interpolator)
        delete m_yaw_interpolator;
}

bool NavFileReader::loadFileToMemory()
{
    m_datetime.clear();
    m_lat.clear();
    m_lon.clear();
    m_depth.clear();
    m_roll.clear();
    m_pitch.clear();
    m_yaw.clear();

    if( !m_file_path.isEmpty() )
    {
        int headerlines = 1;

        QFile file(m_file_path);
        if(!file.open(QIODevice::ReadOnly)) {
            return false;
        }

        QTextStream file_stream(&file);
        int index = 0;

        while(!file_stream.atEnd()) {

            QString line = file_stream.readLine();
            QStringList fields = line.split("\t");

            if (index >headerlines)
            {
                std::pair<double, double> pair;
                QDate date = QDate::fromString(fields[0],"dd/MM/yyyy");
                QTime time = QTime::fromString(fields[1].mid(0,12),"hh:mm:ss.zzz");
                QDateTime date_time(date,time);
                m_datetime.push_back((double)date_time.toMSecsSinceEpoch());
                pair.first = (double)date_time.toMSecsSinceEpoch();

                pair.second = fields[2].toDouble();
                m_lat.push_back(pair);

                pair.second = fields[3].toDouble();
                m_lon.push_back(pair);

                pair.second = -fields[4].toDouble(); // depth is reversed for phins
                m_depth.push_back(pair);

                pair.second = DEG2RAD*fields[5].toDouble();
                m_yaw.push_back(pair);

                pair.second = DEG2RAD*fields[6].toDouble();
                m_pitch.push_back(pair);

                pair.second = DEG2RAD*fields[7].toDouble();
                m_roll.push_back(pair);

            }
            index++;

        }

        file.close();

        m_lat_interpolator = new Interpolator(m_lat);
        m_lon_interpolator = new Interpolator(m_lon);
        m_depth_interpolator = new Interpolator(m_depth);
        m_yaw_interpolator = new AngleInterpolator(m_yaw);
        m_pitch_interpolator = new AngleInterpolator(m_pitch);
        m_roll_interpolator = new AngleInterpolator(m_roll);

        return true;
    }
    else
    {
        return false;
    }
}

bool NavFileReader::isValid() const
{
    return m_is_valid;
}

double NavFileReader::latAtTime(double &_time)
{
    return m_lat_interpolator->findValue(_time);
}

double NavFileReader::lonAtTime(double &_time)
{
    return m_lon_interpolator->findValue(_time);
}

double NavFileReader::depthAtTime(double &_time)
{
    return m_depth_interpolator->findValue(_time);
}

double NavFileReader::rollAtTime(double &_time)
{
    return m_roll_interpolator->findValue(_time);
}

double NavFileReader::pitchAtTime(double &_time)
{
    return m_pitch_interpolator->findValue(_time);
}

double NavFileReader::yawAtTime(double &_time)
{
    return m_yaw_interpolator->findValue(_time);
}
